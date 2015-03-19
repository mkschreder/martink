/* =============================================================================

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
Author(s):	  Pieter Conradie
Creation Date:  2007-03-31
Revision Info:  $Id: twi_master.c 117 2010-06-24 20:21:28Z pieterconradie $

============================================================================= */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>

#include <arch/soc.h>
#include <string.h>
#include <kernel/dev/i2c.h>

#include "twi.h"

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

struct avr_i2c_device {
	struct io_device io; 
	
	uint8_t dev_id; 
	volatile uint8_t addr; // current i2c address
	volatile uint8_t buffer[AVR_I2C_BUFFER_SIZE]; 
	volatile uint8_t cur; // current position in buffer
	volatile uint8_t tr_size; // total size of pending transaction
	volatile uint8_t status; 
	//struct block_device_ops *api; 
	//struct pt *user_thread; 
}; 

static struct avr_i2c_device _device; 

/// TWI state machine interrupt handler
ISR(TWI_vect)
{
	uint8_t status = TWSR; 
	switch(status)
	{
	case TW_START:
		// START has been transmitted
		// Fall through...

	case TW_REP_START:
		// REPEATED START has been transmitted
		_device.cur = 0; 
		
		// Load data register with TWI slave address
		TWDR = _device.addr;
		// TWI Interrupt enabled and clear flag to send next byte
		TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		break;

	case TW_MT_SLA_ACK:
		// SLA+W has been tramsmitted and ACK received
		// Fall through...

	case TW_MT_DATA_ACK:
		// Data byte has been tramsmitted and ACK received
		if(_device.tr_size != 0)
		{
			// Load data register with next byte
			TWDR = _device.buffer[_device.cur];
			// Decrement counter
			_device.tr_size--;
			_device.cur++; 
			// TWI Interrupt enabled and clear flag to send next byte
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		else
		{
			if(_device.status & AVR_I2C_FLAG_SEND_STOP)
				// Initiate STOP condition after last byte; TWI Interrupt disabled
				TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
			else
				// allow rep start
				TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
			
			// Transfer finished
			_device.status |= AVR_I2C_FLAG_DONE;
		}
		break;

	case TW_MR_DATA_ACK:
		// Data byte has been received and ACK tramsmitted
		// Buffer received byte
		_device.buffer[_device.cur] = TWDR;
		// Decrement counter
		_device.tr_size--;
		_device.cur++; 
		// Fall through...

	case TW_MR_SLA_ACK:
		// SLA+R has been transmitted and ACK received
		// See if last expected byte will be received ...
		if(_device.tr_size > 1)
		{
			// Send ACK after reception
			TWCR = (1<<TWINT)|(1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		else
		{
			// Send NACK after next reception
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		break;

	case TW_MR_DATA_NACK:
		// Data byte has been received and NACK tramsmitted
		// Buffer received byte
		_device.buffer[_device.cur] = TWDR;
		// Decrement counter
		_device.tr_size--;
		_device.cur ++; 
		
		if(_device.status & AVR_I2C_FLAG_SEND_STOP)
			// Initiate STOP condition after last byte; TWI Interrupt disabled
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
		else 
			// Allow repeated start! Disable TWI Interrupt
			TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

		// Transfer finished (number of received bytes is in cur) 
		_device.status |= AVR_I2C_FLAG_DONE | AVR_I2C_FLAG_DATA_READY;
		break;

	case TW_MT_ARB_LOST:
		// Arbitration lost...
		// Initiate a (REPEATED) START condition; Interrupt enabled and flag cleared
		TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		break;

	default:
		// Error condition; save status
		//_device.status = TWSR;
		// Reset TWI Interface; disable interrupt
		TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
	}
}

/* _____FUNCTIONS_____________________________________________________ */


static uint8_t _avr_i2c_busy(uint8_t dev_id){
	if(dev_id >= 1) return 1; 
	if(((TWCR & _BV(TWIE)) != 0) || (TWCR & _BV(TWSTO))) return 1; 
	return 0; 
}

int8_t avr_i2c_init(uint8_t dev_id) {
	// only one twi interface for now
	if(dev_id >= 1) return -1; 
	
	io_init(&_device.io); 
	
	// Initialise variable
	_device.tr_size = 0;
	_device.cur = 0; 
	_device.dev_id = 0; 
	
	// Initialize TWI clock
	TWSR = TWI_PRESCALER;
	TWBR = ((F_CPU / TWI_FREQ) - 16) / (2 * 1);

	// Load data register with default content; release SDA
	TWDR = 0xff;

	_device.status |= AVR_I2C_FLAG_ENABLED; 
	_device.status &= ~AVR_I2C_FLAG_SEND_STOP; 
	
	// Enable TWI peripheral with interrupt disabled
	TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

	// set pins to internal pullups
	PORTC |= _BV(5) | _BV(4); 
	
	return 0; 
}

void avr_i2c_deinit(uint8_t dev_id){
	(void)(dev_id); 
	// TODO
}


static PT_THREAD(_avr_i2c_open(struct pt *pt, struct io_device *dev)){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, io); 
	
	PT_BEGIN(pt); 
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	PT_END(pt); 
}

static PT_THREAD(_avr_i2c_close(struct pt *pt, struct io_device *dev)){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, io); 
	
	PT_BEGIN(pt); 
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	I2C_DEBUG("I2C: close\n"); 
	PT_END(pt); 
}

static PT_THREAD(_avr_i2c_seek(struct pt *pt, struct io_device *dev, ssize_t pos, int whence)){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, io); 
	
	PT_BEGIN(pt); 
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	(void)whence; 
	
	I2C_DEBUG("i2c: seek %d\n", pos); 
	self->addr = pos; 
	PT_END(pt); 
}

static PT_THREAD(_avr_i2c_write(struct pt *pt, struct io_device *dev, const uint8_t *data, ssize_t count)){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, io); 
	
	PT_BEGIN(pt);
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	
	// Copy address; clear R/~W bit in SLA+R/W address field
	self->addr &= ~I2C_READ;

	// Copy some data to the buffer
	if(count > AVR_I2C_BUFFER_SIZE) count = AVR_I2C_BUFFER_SIZE; 
	
	I2C_DEBUG("I2C: writestart %x %d: ", self->addr, (int)count); 
	for(int c = 0; c < count; c++) I2C_DEBUG("%x ", data[c]);
	I2C_DEBUG("\n"); 
	
	for(int c = 0; c < count; c++) self->buffer[c] = data[c]; 
	self->tr_size = count; 
	self->cur = 0; 
	
	// Initiate a START condition; Interrupt enabled and flag cleared
	TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
	
	// wait until the operation completes
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	
	PT_END(pt); 
}

static PT_THREAD(_avr_i2c_read(struct pt *pt, struct io_device *dev, uint8_t *data, ssize_t count)){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, io); 
	
	PT_BEGIN(pt); 
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	
	self->addr |= I2C_READ;

	if(count > AVR_I2C_BUFFER_SIZE) count = AVR_I2C_BUFFER_SIZE; 
	self->tr_size = count;
	self->cur = 0; 
	
	I2C_DEBUG("I2C: readstart: %x %d\n", self->addr, (int)count); 
	
	// Initiate a START condition; Interrupt enabled and flag cleared
	TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
	
	// wait until the read is completed
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id)); 
	
	// copy data to the supplied buffer
	if(self->cur < count) count = self->cur; 
	for(int c = 0; c < count; c++) data[c] = self->buffer[c]; 
	I2C_DEBUG("I2C: readready: %x %d: ", self->addr, (int)count); 
	for(int c = 0; c < count; c++) I2C_DEBUG("%x ", self->buffer[c]);
	I2C_DEBUG("\n"); 
	
	PT_END(pt); 
}

static PT_THREAD(_avr_i2c_ioctl(struct pt *pt, struct io_device *dev, ioctl_req_t req, va_list vl)){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, io); 
	
	PT_BEGIN(pt); 
	PT_WAIT_WHILE(pt, _avr_i2c_busy(self->dev_id));
	
	if(req == I2C_SEND_STOP){
		uint8_t en = va_arg(vl, int); 
		I2C_DEBUG("I2C: sendstop: %d\n", en); 
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if(en)
				self->status |= AVR_I2C_FLAG_SEND_STOP; 
			else
				self->status &= ~AVR_I2C_FLAG_SEND_STOP; 
		}
	} 
	
	PT_END(pt); 
}

io_dev_t avr_i2c_get_interface(uint8_t dev_id){
	if(dev_id > 0) return 0; 
	
	static struct io_device_ops _if;
	if(!_device.io.api){
		_if = (struct io_device_ops) {
			.open = _avr_i2c_open, 
			.close = _avr_i2c_close, 
			.read = 	_avr_i2c_read,
			.write = 	_avr_i2c_write,
			.seek = _avr_i2c_seek, 
			.ioctl = _avr_i2c_ioctl, 
		};
		_device.io.api = &_if; 
	}
	return &_device.io; 
}

/*
void twi_wait(uint8_t dev_id, uint8_t addr){
	// not implemented
	(void)(dev_id); 
	(void)(addr); 
}
*/
/*
void twi0_slave_init(uint8_t addr){
	TWAR = addr; 
  TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC); 
  //TWI_busy = 0;
}*/

