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
	uint8_t dev_id; 
	volatile uint8_t addr; // current i2c address
	volatile uint8_t buffer[AVR_I2C_BUFFER_SIZE]; 
	volatile uint8_t cur; 
	volatile uint8_t tr_size; 
	volatile uint8_t status; 
	struct block_device_ops *api; 
	struct pt *user_thread; 
	/*
	uint8_t *twi_rd_data;
	const uint8_t *twi_wr_data; 
	static volatile uint8_t twi_data_counter;
	static volatile uint8_t twi_status;*/
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
			// Allow rep start! Disable TWI Interrupt
			if(_device.status & AVR_I2C_FLAG_SEND_STOP)
				// Initiate STOP condition after last byte; TWI Interrupt disabled
				TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
			else
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
	return (((TWCR & _BV(TWIE)) != 0) || (TWCR & _BV(TWSTO)))?1:0;
}

static int8_t _avr_i2c_stop(struct avr_i2c_device *self){
	(void)self; 
	if(_avr_i2c_busy(self->dev_id)) return -EBUSY; 
	//if(!(self->status & AVR_I2C_FLAG_STOP_REQUIRED)) return 0; 
	//self->status &= ~AVR_I2C_FLAG_STOP_REQUIRED; 
	// not sure if this is really necessary
	/* if(!(self->status & AVR_I2C_FLAG_DONE)) return -EBUSY; 
	*/
	//I2C_DEBUG("I2C: stop\n"); 
	// Initiate a STOP condition
	//TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
	//while(_avr_i2c_busy(self->dev_id)); 
	
	return 0; 
}

int8_t avr_i2c_init(uint8_t dev_id) {
	// only one twi interface for now
	if(dev_id >= 1) return -1; 
	
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


static uint8_t _avr_i2c_open(block_dev_t dev){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, api); 
	if(_avr_i2c_busy(self->dev_id)) return 0; 
	if(self->status & AVR_I2C_FLAG_LOCKED) return 0; 
	
	I2C_DEBUG("I2C: open\n"); 
	
	self->status |= AVR_I2C_FLAG_LOCKED; 
	self->user_thread = libk_current_thread(); 
	
	return 1; 
}

static int8_t _avr_i2c_close(block_dev_t dev){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, api); 
	if(self->user_thread != libk_current_thread()) return -1; 
	if(!(self->status & AVR_I2C_FLAG_LOCKED)) return -1; 
	
	_avr_i2c_stop(self); 
	
	I2C_DEBUG("I2C: close\n"); 
	
	self->status &= ~AVR_I2C_FLAG_LOCKED; 
	self->user_thread = 0; 
	
	return 0; 
}

static ssize_t _avr_i2c_seek(block_dev_t dev, ssize_t pos, int whence){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	(void)whence; 
	
	I2C_DEBUG("i2c: seek %d\n", pos); 
	self->addr = pos; 
	return self->addr; 
	/*
	switch(whence){
		case SEEK_SET: self->addr = pos; return self->addr; 
		case SEEK_CUR: self->addr += pos; return self->addr; 
		default: 
			return -1; 
	}
	return -1; */
}

static ssize_t _avr_i2c_write(block_dev_t dev, const uint8_t *data, ssize_t count){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	// all io is non blocking in libk!
	if(_avr_i2c_busy(self->dev_id)) {
		I2C_DEBUG("i2c: writebusy\n"); 
		return -EWOULDBLOCK; 
	}
	
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
	
	// return number of bytes we have stashed for transmission
	return count; 
}

static ssize_t _avr_i2c_read(block_dev_t dev, uint8_t *data, ssize_t count){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	// Wait for previous transaction to finish
	if(_avr_i2c_busy(self->dev_id)) {
		//I2C_DEBUG("i2cbusy\n"); 
		return -EWOULDBLOCK; 
	}
	
	// check if any data has been stored in the receive buffer and return it
	if(self->status & AVR_I2C_FLAG_DATA_READY){
		if(self->cur < count) count = self->cur; 
		for(int c = 0; c < count; c++) data[c] = self->buffer[c]; 
		I2C_DEBUG("I2C: readready: %x %d: ", self->addr, (int)count); 
		for(int c = 0; c < count; c++) I2C_DEBUG("%x ", self->buffer[c]);
		I2C_DEBUG("\n"); 
	
		self->status &= ~AVR_I2C_FLAG_DATA_READY; 
		return count; 
	}
	
	// Copy address; set R/~W bit in SLA+R/W address field
	self->addr |= I2C_READ;
	
	if(count > AVR_I2C_BUFFER_SIZE) count = AVR_I2C_BUFFER_SIZE; 
	self->tr_size = count;
	self->cur = 0; 
	
	I2C_DEBUG("I2C: readstart: %x %d\n", self->addr, (int)count); 
	
	// Initiate a START condition; Interrupt enabled and flag cleared
	TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
	
	// signal that user should call this function again
	return -EAGAIN; 
}
/*
uint8_t _avr_i2c_status(uint8_t dev_id, uint16_t flags){
	if(dev_id >= 1) return 1; 
	uint8_t ret = 0; 
	// IF TWI Interrupt is enabled then the peripheral is busy
	if(flags & I2CDEV_BUSY) ret |= (((TWCR & _BV(TWIE)) != 0) || (TWCR & _BV(TWSTO)))?1:0; 
	return ret; 
}*/

static int16_t _avr_i2c_ioctl(block_dev_t dev, ioctl_req_t req, ...){
	struct avr_i2c_device *self = container_of(dev, struct avr_i2c_device, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	switch(req) {
		case I2C_SEND_STOP: {
			va_list vl; 
			va_start(vl, req); 
			uint8_t en = va_arg(vl, int); 
			va_end(vl); 
			I2C_DEBUG("I2C: sendstop: %d\n", en); 
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				if(en)
					self->status |= AVR_I2C_FLAG_SEND_STOP; 
				else
					self->status &= ~AVR_I2C_FLAG_SEND_STOP; 
			}
		} return 0; 
			//return _avr_i2c_stop(self); 
			// Wait until STOP has finished
			//while(TWCR & _BV(TWSTO));
		default: 
			return -EINVAL; 
	}
	return -1; 
}

block_dev_t avr_i2c_get_interface(uint8_t dev_id){
	if(dev_id > 0) return 0; 
	
	static struct block_device_ops _if;
	_if = (struct block_device_ops) {
		.open = _avr_i2c_open, 
		.close = _avr_i2c_close, 
		.read = 	_avr_i2c_read,
		.write = 	_avr_i2c_write,
		.seek = _avr_i2c_seek, 
		.ioctl = _avr_i2c_ioctl, 
	};
	_device.api = &_if; 
	return &_device.api; 
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

