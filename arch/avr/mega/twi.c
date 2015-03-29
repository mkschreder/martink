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
#include <kernel/cbuf.h>

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
	
	struct io_device io; 
	struct cbuf buffer; 
	
	uint8_t _buffer[AVR_I2C_BUFFER_SIZE]; 
	
	volatile uint8_t addr; // current i2c address
	volatile ssize_t rx_left; 
	volatile ssize_t queued_count; 
	volatile uint8_t status; 
	
	async_mutex_t lock, buffer_lock; 
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
		//_device.cur = 0; 
		
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
		if(cbuf_get_waiting_isr(&_device.buffer))
		{
			// Load data register with next byte
			TWDR = cbuf_get_isr(&_device.buffer);
			
			// TWI Interrupt enabled and clear flag to send next byte
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		else
		{
			// Transfer finished
			_device.status |= AVR_I2C_FLAG_DONE;
			
			if(_device.status & AVR_I2C_FLAG_SEND_STOP)
				// Initiate STOP condition after last byte; TWI Interrupt disabled
				TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
			else
				// allow rep start
				TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
			
		}
		break;

	case TW_MR_DATA_ACK:
		// Data byte has been received and ACK tramsmitted
		// Buffer received byte
		cbuf_put_isr(&_device.buffer, TWDR);
		_device.rx_left--; 
		// Fall through...

	case TW_MR_SLA_ACK:
		// SLA+R has been transmitted and ACK received
		// See if last expected byte will be received ...
		if(_device.rx_left > 1) {
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
		cbuf_put_isr(&_device.buffer, TWDR);
		// Decrement counter
		_device.rx_left--;
		
		// Transfer finished (number of received bytes is in cur) 
		_device.status |= AVR_I2C_FLAG_DONE | AVR_I2C_FLAG_DATA_READY;
		
		if(_device.status & AVR_I2C_FLAG_SEND_STOP)
			// Initiate STOP condition after last byte; TWI Interrupt disabled
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
		else 
			// Allow repeated start! Disable TWI Interrupt
			TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

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
	
	struct avr_i2c_device *self = &_device; 
	
	io_init(&self->io); 
	cbuf_init(&self->buffer, self->_buffer, AVR_I2C_BUFFER_SIZE); 
	
	ASYNC_MUTEX_INIT(self->lock, 1); 
	ASYNC_MUTEX_INIT(self->buffer_lock, 1); 
	
	// Initialise variable
	self->rx_left = 0; 
	self->status = AVR_I2C_FLAG_ENABLED; 
	self->status &= ~AVR_I2C_FLAG_SEND_STOP; 
	
	// Initialize TWI clock
	TWSR = TWI_PRESCALER;
	TWBR = ((F_CPU / TWI_FREQ) - 16) / (2 * 1);

	// Load data register with default content; release SDA
	TWDR = 0xff;

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


static ASYNC(io_result_t, io_device_t, vopen){
	struct avr_i2c_device *dev = container_of(self, struct avr_i2c_device, io); 
	
	ASYNC_BEGIN(); 
	
	ASYNC_MUTEX_LOCK(dev->lock); 
	
	AWAIT(!_avr_i2c_busy(dev->dev_id)); 
	
	I2C_DEBUG("i2c: open\n"); 
	ASYNC_END(0); 
}

static ASYNC(io_result_t, io_device_t, vclose){
	struct avr_i2c_device *dev = container_of(self, struct avr_i2c_device, io); 
	
	ASYNC_BEGIN(); 
	AWAIT(!_avr_i2c_busy(dev->dev_id)); 
	
	ASYNC_MUTEX_UNLOCK(dev->lock); 
	
	I2C_DEBUG("I2C: closed\n"); 
	ASYNC_END(0); 
}

static ASYNC(io_result_t, io_device_t, vseek, ssize_t pos, int whence){
	struct avr_i2c_device *dev = container_of(self, struct avr_i2c_device, io); 
	
	ASYNC_BEGIN();
	
	AWAIT(!_avr_i2c_busy(dev->dev_id)); 
	(void)whence; 
	
	I2C_DEBUG("i2c: seek %d\n", pos); 
	dev->addr = pos; 
	
	ASYNC_END(pos); 
}

static ASYNC(io_result_t, io_device_t, vwrite, const uint8_t *data, ssize_t size){
	struct avr_i2c_device *dev = container_of(self, struct avr_i2c_device, io); 
	
	ASYNC_BEGIN();
		// lock the buffer so that read can not start using it
		ASYNC_MUTEX_LOCK(dev->buffer_lock); 
		
		// wait for previous transaction to complete
		AWAIT(!_avr_i2c_busy(dev->dev_id)); 
		
		// set address. clear R/~W bit in SLA+R/W address field
		dev->addr &= ~I2C_READ;
		
		// clear buffer
		cbuf_clear(&dev->buffer); 
		dev->queued_count = 0; 
		
		I2C_DEBUG("I2C: wstart %x %d: ", dev->addr, (int)size); 
		for(int c = 0; c < size; c++) I2C_DEBUG("%x ", data[c]);
		I2C_DEBUG("\n"); 
		
		// we do a busy loop here for pushing data as quickly as possible
		// into the cache. This is because it is tricky to make i2c isr wait
		// for more data. Instead it is better to busy loop until all data is
		// in cache and then do an async wait until transaction completes. 
		// this means that i2c transactions < cache size are async. All others
		// are blocking for as long as necessary to make sure that data gets
		// cached properly. 
		while(dev->queued_count < size){
			// we try to write to the data queue 
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				dev->queued_count += cbuf_putn(&dev->buffer, 
					data + dev->queued_count, size - dev->queued_count); 
			}
			I2C_DEBUG("i2c: wrote %d\n", dev->queued_count); 
			// start transmission if it has not been started yet
			if(!_avr_i2c_busy(dev->dev_id))
				TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		
		// here we can yield because now all data is in the buffer 
		AWAIT(!_avr_i2c_busy(dev->dev_id)); 
		
		ASYNC_MUTEX_UNLOCK(dev->buffer_lock); 
	ASYNC_END(size); 
}

static ASYNC(io_result_t, io_device_t, vread, uint8_t *data, ssize_t size){
	struct avr_i2c_device *dev = container_of(self, struct avr_i2c_device, io); 
	
	ASYNC_BEGIN(); 
		// lock the buffer so that read can not start using it
		// must always lock mutex first before checking for busy device
		ASYNC_MUTEX_LOCK(dev->buffer_lock); 
		
		AWAIT(!_avr_i2c_busy(dev->dev_id)); 
			
		dev->addr |= I2C_READ;

		// clear buffer
		cbuf_clear(&dev->buffer); 
		dev->queued_count = 0; 
		dev->rx_left = size; 
		
		I2C_DEBUG("I2C: readstart: %x %d\n", dev->addr, (int)size); 
		
		// Initiate a START condition; Interrupt enabled and flag cleared
		TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		
		// here we make sure that we receive the whole buffer before returning
		// if the buffer becomes full before we are able to copy data, data will
		// be lost. This means that we can only reliably receive one i2c block
		// at a time. 
		while(dev->queued_count < size){
			AWAIT(!cbuf_is_empty(&dev->buffer)); 
			
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
				dev->queued_count += cbuf_getn(&dev->buffer, 
					data + dev->queued_count, size - dev->queued_count); 
			}
		}
		
		// always for safety make sure that everything is done
		AWAIT(!_avr_i2c_busy(dev->dev_id)); 
		
		I2C_DEBUG("I2C: readready: %x %d: ", dev->addr, (int)size); 
		for(int c = 0; c < size; c++) I2C_DEBUG("%x ", data[c]);
		I2C_DEBUG("\n"); 
		
		ASYNC_MUTEX_UNLOCK(dev->buffer_lock); 
		
	ASYNC_END(size); 
}

static ASYNC(io_result_t, io_device_t, vioctl, ioctl_req_t req, va_list vl){
	struct avr_i2c_device *dev = container_of(self, struct avr_i2c_device, io); 
	
	ASYNC_BEGIN(); 
	
	// wait for the device to not be busy since changing settings on a busy
	// device is dangerous
	AWAIT(!_avr_i2c_busy(dev->dev_id));
	
	if(req == I2C_SEND_STOP){
		uint8_t en = va_arg(vl, int); 
		I2C_DEBUG("I2C: sendstop: %d\n", en); 
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			if(en)
				dev->status |= AVR_I2C_FLAG_SEND_STOP; 
			else
				dev->status &= ~AVR_I2C_FLAG_SEND_STOP; 
		}
		ASYNC_EXIT(0); 
	} 
	
	ASYNC_END(-1); 
}

io_dev_t avr_i2c_get_interface(uint8_t dev_id){
	if(dev_id > 0) return 0; 
	
	static struct io_device_ops _if;
	if(!_device.io.api){
		_if = (struct io_device_ops) {
			.open = __io_device_t_vopen__, 
			.close = __io_device_t_vclose__, 
			.read = 	__io_device_t_vread__,
			.write = 	__io_device_t_vwrite__,
			.seek = __io_device_t_vseek__, 
			.ioctl = __io_device_t_vioctl__, 
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
