/* =============================================================================

Copyright (C) 2016 Martin Schröder <mkschreder.uk@gmail.com>
Copyright (c) 2006 Pieter Conradie [www.piconomic.co.za]
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.

* Neither the name of the copyright holders nor the names of
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

Title:		  Interrupt-driven TWI (I2C) Master
Author(s):	  Pieter Conradie, Martin Schröder
Creation Date:  2007-03-31
Revision Info:  $Id: twi_master.c 117 2010-06-24 20:21:28Z pieterconradie $

============================================================================= */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>

#include <arch/soc.h>
#include <string.h>
#include <kernel/mt.h>
#include <kernel/cbuf.h>

#include "i2c.h"

#define hwtwi0_init(speed) {\
	TWSR = 0;\
  TWBR = (uint8_t)(((F_CPU/speed)-16)/2);\
}

#define TWCR_DEFAULT (_BV(TWEN) | _BV(TWIE)) //_BV(TWEA)
#define TWI_ERR_BUSY 				0x0100
#define TWI_ERR_NO_RESPONSE 0x0200

#define TWI_READY 0
#define TWI_IDLE	1
#define TWI_NO_RESPONSE 2
#define TWI_REP_START_SENT 4

#define AVR_I2C_ADDR_SIZE 1
#define AVR_I2C_BUFFER_SIZE (8 + AVR_I2C_ADDR_SIZE)

#define TWI_FREQ 100000

/// Size TWI clock prescaler according to desired TWI clock frequency
#define TWI_PRESCALER	((0<<TWPS1)|(0<<TWPS0))

/// Calculate TWI Baud rate value according to selected frequency and prescaler
#define TWI_BR_VALUE ((DIV_ROUND(F_CPU,TWI_FREQUENCY_HZ)-16ul)/(2ul*TWI_PRESCALER_VALUE))

/// TWI State machine value when finished
//#define TWI_STATUS_DONE 0xff
#define AVR_I2C_FLAG_DONE (1 << 0) // the device is ready to accept more data
#define AVR_I2C_FLAG_DATA_READY (1 << 1) // read data in buffer
#define AVR_I2C_FLAG_ENABLED (1 << 2) 
#define AVR_I2C_FLAG_LOCKED (1 << 3)
#define AVR_I2C_FLAG_SEND_STOP (1 << 4)

#define I2C_DEBUG(...) {} //printf(__VA_ARGS__)

#define twi0_send_start() do { TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE); } while(0)  
#define twi0_interrupt_on() do { TWCR |= _BV(TWIE); } while(0)
#define twi0_interrupt_off() do { TWCR &= ~_BV(TWIE); } while(0)
#define twi0_send_stop() do { TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE); } while(0)
#define twi0_wait_ready() while(TWCR & _BV(TWIE)) 
#define twi0_wait_stop() while(TWCR & _BV(TWSTO))

struct avr_i2c_adapter {
	struct i2c_adapter adapter; 

	volatile struct i2c_client *client; // current client using the driver
	volatile const char *wr_data; 
	volatile char *rd_data; 
	volatile uint8_t size;

	volatile uint8_t addr; // current i2c address
	volatile int status; 

	mutex_t lock; 
	sem_t ready; 
}; 

static struct avr_i2c_adapter twi0; 

/// TWI state machine interrupt handler
ISR(TWI_vect){
	uint8_t status = TWSR; 
	struct avr_i2c_adapter *self = &twi0; 
	if(!self->client) return; // TODO: maybe signal this somehow? client MUST always be set when we get here!

	switch(status){
	case TW_START:
		// START has been transmitted
		// Fall through...
	case TW_REP_START:
		// TODO: what happens if we get here because of arbitration lost? 
		// REPEATED START has been transmitted
		// Load data register with TWI slave address
		TWDR = self->client->addr;
		// TWI Interrupt enabled and clear flag to send next byte
		TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		break;
	case TW_MT_SLA_ACK:
		// SLA+W has been tramsmitted and ACK received
		// Fall through...
	case TW_MT_DATA_ACK:
		// Data byte has been tramsmitted and ACK received
		if(self->size > 0){
			// Load data register with next byte
			TWDR = *(self->wr_data);
			self->wr_data++; self->size--; 
			// TWI Interrupt enabled and clear flag to send next byte
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		} else {	
			// allow rep start
			TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

			// Transfer finished
			self->status = 0; 
			sem_give_from_isr(&self->ready); 
		}
		break;
	case TW_MR_DATA_ACK:
		// Data byte has been received and ACK tramsmitted
		// Buffer received byte
		*(self->rd_data) = TWDR; 
		self->rd_data++; self->size--; 
		// Fall through...
	case TW_MR_SLA_ACK:
		// SLA+R has been transmitted and ACK received
		// See if last expected byte will be received ...
		if(self->size > 1) {
			// Send ACK after reception
			TWCR = (1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		} else {
			// Send NACK after next reception
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		break;
	case TW_MR_DATA_NACK:
		// Data byte has been received and NACK tramsmitted
		// Buffer received byte
		*(self->rd_data) = TWDR; 
		self->size--; self->rd_data++;   

		// Allow repeated start! Disable TWI Interrupt
		TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
	
		// Transfer finished 
		twi0.status = 0; 
		sem_give_from_isr(&self->ready); 

		break;
	case TW_MT_ARB_LOST:
		// Arbitration lost...
		// Initiate a (REPEATED) START condition; Interrupt enabled and flag cleared
		TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		break;

	default:
		// Error condition; save status
		//twi0.status = TWSR;
		// Reset TWI Interface; disable interrupt
		TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

		twi0.status = -EFAULT; 
		sem_give_from_isr(&self->ready); 
	}
}

/* _____FUNCTIONS_____________________________________________________ */

static int _atmega_i2c_write(struct avr_i2c_adapter *self, struct i2c_client *client, const char *data, size_t size){
	twi0_wait_ready(); 
	client->addr &= ~I2C_READ; 	
	self->status = 0; 
	self->wr_data = data; 
	self->size = size; 
	twi0_send_start(); 
	sem_take(&self->ready); 
	return (twi0.status < 0)?twi0.status:((int)size - self->size); 
}
static int _atmega_i2c_read(struct avr_i2c_adapter *self, struct i2c_client *client, char *data, size_t size){
	twi0_wait_ready(); 
	client->addr |= I2C_READ; 
	self->status = 0; 
	self->rd_data = data; 
	self->size = size; 
	twi0_send_start(); 
	sem_take(&self->ready); 
	return (twi0.status < 0)?twi0.status:((int)size - self->size); 
}

static int _atmega_i2c_stop(struct avr_i2c_adapter *self, struct i2c_client *client){
	twi0_send_stop(); 
	twi0_wait_stop(); 
	return 0; 
}

static int atmega_i2c_write(struct i2c_client *client, const char *data, size_t size){
	struct avr_i2c_adapter *self = container_of(client->adapter, struct avr_i2c_adapter, adapter); 
	mutex_lock(&self->lock); 
	self->client = client; 
	int ret = _atmega_i2c_write(self, client, data, size); 	
	_atmega_i2c_stop(self, client); 
	mutex_unlock(&self->lock); 
	return ret; 
}

static int atmega_i2c_read(struct i2c_client *client, char *data, size_t size){
	struct avr_i2c_adapter *self = container_of(client->adapter, struct avr_i2c_adapter, adapter); 
	mutex_lock(&self->lock); 
	self->client = client; 
	int ret = _atmega_i2c_read(self, client, data, size); 	
	_atmega_i2c_stop(self, client); 
	mutex_unlock(&self->lock); 
	return ret; 
}

#include <serial/serial.h>
static int atmega_i2c_transfer(struct i2c_client *client, const char *outbuf, size_t out_size, char *in_buf, size_t in_size){
	if(!client || !outbuf || !in_buf) return -EINVAL; 
	struct avr_i2c_adapter *self = container_of(client->adapter, struct avr_i2c_adapter, adapter); 
	mutex_lock(&self->lock); 
	self->client = client; 
	int ret = _atmega_i2c_write(self, client, outbuf, out_size); 
	if(ret < 0) goto error; 
	ret = _atmega_i2c_read(self, client, in_buf, in_size); 
	if(ret < 0) goto error; 
	_atmega_i2c_stop(self, client); 
	mutex_unlock(&self->lock); 
	return ret; 
error: 
	_atmega_i2c_stop(self, client); 
	mutex_unlock(&self->lock); 
	return ret; 
}

static struct i2c_adapter_ops atmega_i2c_ops = {
	.read = atmega_i2c_read, 
	.write = atmega_i2c_write, 
	.transfer = atmega_i2c_transfer
}; 

static void __init atmega_i2c_init(void){
	// only one twi interface for now
	memset(&twi0, 0, sizeof(twi0)); 
	struct avr_i2c_adapter *self = &twi0; 

	// Initialise variable
	mutex_init(&self->lock); 
	sem_init(&self->ready, 1, 0); 
	
	i2c_register_adapter(&self->adapter); 

	// Initialize TWI clock
	TWSR = 0; //TWI_PRESCALER;
	TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

	// Load data register with default content; release SDA
	TWDR = 0xff;

	// Enable TWI peripheral with interrupt disabled
	TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

	// set pins to internal pullups
	PORTC |= _BV(5) | _BV(4); 

	self->adapter.ops = &atmega_i2c_ops; 
	i2c_register_adapter(&twi0.adapter); 
}

