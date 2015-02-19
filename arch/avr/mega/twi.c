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

#include "twi.h"

#define TWI_FREQ 100000

/// Size TWI clock prescaler according to desired TWI clock frequency
#define TWI_PRESCALER	((0<<TWPS1)|(0<<TWPS0))

/// Calculate TWI Baud rate value according to selected frequency and prescaler
#define TWI_BR_VALUE ((DIV_ROUND(F_CPU,TWI_FREQUENCY_HZ)-16ul)/(2ul*TWI_PRESCALER_VALUE))

/// TWI State machine value when finished
#define TWI_STATUS_DONE 0xff

static volatile uint8_t twi_adr;
static volatile uint8_t *twi_rd_data;
static volatile const uint8_t *twi_wr_data; 
static volatile uint8_t twi_data_counter;
static volatile uint8_t twi_status;

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

		// Load data register with TWI slave address
		TWDR = twi_adr;
		// TWI Interrupt enabled and clear flag to send next byte
		TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		break;

	case TW_MT_SLA_ACK:
		// SLA+W has been tramsmitted and ACK received
		// Fall through...

	case TW_MT_DATA_ACK:
		// Data byte has been tramsmitted and ACK received
		if(twi_data_counter != 0)
		{
			// Decrement counter
			twi_data_counter--;
			// Load data register with next byte
			TWDR = *twi_wr_data++;
			// TWI Interrupt enabled and clear flag to send next byte
			TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		}
		else
		{
			// Allow rep start! Disable TWI Interrupt
			TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

			// Initiate STOP condition after last byte; TWI Interrupt disabled
			//TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

			// Transfer finished
			twi_status = TWI_STATUS_DONE;
		}
		break;

	case TW_MR_DATA_ACK:
		// Data byte has been received and ACK tramsmitted
		// Buffer received byte
		*twi_rd_data++ = TWDR;
		// Decrement counter
		twi_data_counter--;
		// Fall through...

	case TW_MR_SLA_ACK:
		// SLA+R has been transmitted and ACK received
		// See if last expected byte will be received ...
		if(twi_data_counter > 1)
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
		*twi_rd_data++ = TWDR;
		// Decrement counter
		twi_data_counter--;

		// Allow repeated start! Disable TWI Interrupt
		TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

		// Initiate STOP condition after last byte; TWI Interrupt disabled
		//TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

		// Transfer finished
		twi_status = TWI_STATUS_DONE;
		break;

	case TW_MT_ARB_LOST:
		// Arbitration lost...
		// Initiate a (REPEATED) START condition; Interrupt enabled and flag cleared
		TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
		break;

	default:
		// Error condition; save status
		twi_status = TWSR;
		// Reset TWI Interface; disable interrupt
		TWCR = (0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);
	}
}

/* _____FUNCTIONS_____________________________________________________ */
int8_t twi_init(uint8_t dev_id)
{
	// only one twi interface for now
	if(dev_id >= 1) return -1; 
	
	// Initialise variable
	twi_data_counter = 0;

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

void twi_deinit(uint8_t dev_id){
	(void)(dev_id); 
	// TODO
}

int8_t twi_start_write(uint8_t dev_id, uint8_t adr, const uint8_t *data, uint8_t bytes_to_send)
{
	if(dev_id >= 1) return -1; 
	
	// Wait for previous transaction to finish
	while(twi_busy(dev_id)); 

	// Copy address; clear R/~W bit in SLA+R/W address field
	twi_adr = adr & ~I2C_READ;

	// Save pointer to data and number of bytes to send
	twi_wr_data		= data;
	twi_data_counter = bytes_to_send;

	// Initiate a START condition; Interrupt enabled and flag cleared
	TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
	return 0; 
}

int8_t twi_start_read(uint8_t dev_id, uint8_t adr, uint8_t *data, uint8_t bytes_to_receive)
{
	if(dev_id >= 1) return -1; 
	
	// Wait for previous transaction to finish
	while(twi_busy(dev_id)); 

	// Copy address; set R/~W bit in SLA+R/W address field
	twi_adr = adr | I2C_READ;

	// Save pointer to data and number of bytes to receive
	twi_rd_data		 = data;
	twi_data_counter = bytes_to_receive;

	// Initiate a START condition; Interrupt enabled and flag cleared
	TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(1<<TWIE);
	return 0; 
}

uint8_t twi_busy(uint8_t dev_id){
	if(dev_id >= 1) return 1; 
	// IF TWI Interrupt is enabled then the peripheral is busy
	return ((TWCR & _BV(TWIE)) != 0) || (TWCR & _BV(TWSTO)); 
}

int8_t twi_stop(uint8_t dev_id)
{
	if(dev_id >= 1) return -1; 
	
	// Wait for transaction to finish
	while(twi_busy(dev_id));

	// Make sure transaction was succesful
	if(twi_status != TWI_STATUS_DONE)
		return 0;

	// Initiate a STOP condition
	TWCR = (1<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|(0<<TWWC)|(1<<TWEN)|(0<<TWIE);

	// Wait until STOP has finished
	//while(TWCR & _BV(TWSTO));
	
	return twi_status == TWI_STATUS_DONE; 
}

void twi_wait(uint8_t dev_id, uint8_t addr){
	// not implemented
	(void)(dev_id); 
	(void)(addr); 
}

/*
void twi0_slave_init(uint8_t addr){
	TWAR = addr; 
  TWCR = (1<<TWEN)|(0<<TWIE)|(0<<TWINT)|(0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|(0<<TWWC); 
  //TWI_busy = 0;
}*/

