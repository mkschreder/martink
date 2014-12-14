/*************************************************************************
* Title:    I2C master library using hardware TWI interface
* Author:   Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
* File:     $Id: twimaster.c,v 1.3 2005/07/02 11:14:21 Peter Exp $
* Software: AVR-GCC 3.4.3 / avr-libc 1.2.3
* Target:   any AVR device with hardware TWI 
* Usage:    API compatible with I2C Software Library i2cmaster.h
**************************************************************************/
#include <inttypes.h>
#include <compat/twi.h>
#include <util/delay.h>

#include <arch/soc.h>

#include "i2cmaster.h"

#include <assert.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include <stdio.h>
#include <stdlib.h>

#include "i2c.h"

// By default, the control register is set to:
// - TWEA: Automatically send acknowledge bit in receive mode.
// - TWEN: Enable the I2C system.
// - TWIE: Enable interrupt requests when TWINT is set.

#define TWCR_DEFAULT (_BV(TWEA) | _BV(TWEN) | _BV(TWIE))

#define TWCR_NOT_ACK (_BV(TWINT) | _BV(TWEN) | _BV(TWIE))
#define TWCR_ACK (TWCR_NOT_ACK | _BV(TWEA))

#define I2C_FREQ 100000

#define I2C_TXN_DONE _BV(0)
#define I2C_TXN_ERR  _BV(1)

static inline void i2c_op_init(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  o->address = address;
  o->buflen = buflen;
  o->bufpos = 0;
  o->buf = buf;
}

static inline void i2c_op_init_rd(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  i2c_op_init(o, (address << 1) | TW_READ, buf, buflen);
}

static inline void i2c_op_init_wr(i2c_op_t *o, uint8_t address, uint8_t *buf, uint8_t buflen) {
  i2c_op_init(o, (address << 1) | TW_WRITE, buf, buflen);
}

static volatile i2c_txn_t _txn; 
static volatile i2c_txn_t *txn = 0;
static volatile i2c_op_t *op;

void __twi0_init__(void) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		TWSR &= ~(_BV(TWPS1) | _BV(TWPS0));
		TWBR = ((F_CPU / I2C_FREQ) - 16) / (2 * 1);
		
		//PORTC |= _BV(PC5) | _BV(PC4);
		
		TWCR = TWCR_DEFAULT;

		TWAR = 0;
	}
}

uint8_t __twi0_is_busy__(void){
	return txn != 0; 
}

void __twi0_start_transaction__(i2c_op_list_t ops) {
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		if (txn == NULL) { // if no transaction is in progress
			txn = &_txn; 
			
			txn->opslen = 0; 
			while(ops[txn->opslen].buf != 0){
				txn->ops[txn->opslen] = ops[txn->opslen]; 
				txn->opslen++; 
			}
			txn->flags = 0;
			txn->opspos = 0;
			op = &txn->ops[0];

			TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
		} else {
			// throw away??
		}
	}
}

ISR(TWI_vect) {
  uint8_t status = TW_STATUS;

  if ((op->address & _BV(0)) == TW_READ) {
    // Master Receiver mode.
    switch (status) {

    // A START condition has been transmitted. 
    case TW_START:
    // A repeated START condition has been transmitted. 
    case TW_REP_START:
      assert(op->buflen > 0);
      op->bufpos = 0;
      TWDR = op->address;
      TWCR = TWCR_DEFAULT | _BV(TWINT);
      break;

    // Arbitration lost in SLA+R or NOT ACK bit. 
    case TW_MR_ARB_LOST:
      // A START condition will be transmitted when the bus becomes free. 
      TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
      break;

    // SLA+R has been transmitted; ACK has been received. 
    case TW_MR_SLA_ACK:
      if (op->buflen == 1) {
        TWCR = TWCR_NOT_ACK;
      } else {
        TWCR = TWCR_ACK;
      }
      break;

    // SLA+R has been transmitted; NOT ACK has been received. 
    case TW_MR_SLA_NACK:
      txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
      break; 
      //goto next_txn;

    // Data byte has been received; ACK has been returned. 
    case TW_MR_DATA_ACK:
      op->buf[op->bufpos++] = TWDR;
      if (op->bufpos+1 == op->buflen) {
        TWCR = TWCR_NOT_ACK;
      } else {
        TWCR = TWCR_ACK;
      }
      break;

    // Data byte has been received; NOT ACK has been returned. 
    case TW_MR_DATA_NACK:
      op->buf[op->bufpos++] = TWDR;
      goto next_op;

    default:
			assert(0 && "unknown status in master receiver mode");
    }
  } else {
    // Master Transmitter mode. 
    switch (status) {

    // A START condition has been transmitted. 
    case TW_START:
    // A repeated START condition has been transmitted. 
    case TW_REP_START:
      assert(op->buflen > 0);
      op->bufpos = 0;
      TWDR = op->address;
      TWCR = TWCR_DEFAULT | _BV(TWINT);
      break;

    // Arbitration lost in SLA+W or data bytes. 
    case TW_MT_ARB_LOST:
      // A START condition will be transmitted when the bus becomes free. 
      TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
      break;

    // SLA+W has been transmitted; ACK has been received. 
    case TW_MT_SLA_ACK:
      TWDR = op->buf[op->bufpos++];
      TWCR = TWCR_DEFAULT | _BV(TWINT);
      break;

    // SLA+W has been transmitted; NOT ACK has been received. 
    case TW_MT_SLA_NACK:
      txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
      break; 
      //goto next_txn;

    // Data byte has been transmitted; ACK has been received. 
    case TW_MT_DATA_ACK:
      if (op->bufpos < op->buflen) {
        TWDR = op->buf[op->bufpos++];
        TWCR = TWCR_DEFAULT | _BV(TWINT);
        break;
      }

      // No more bytes left to transmit... 
      goto next_op;

    // Data byte has been transmitted; NOT ACK has been received. 
    case TW_MT_DATA_NACK:
      if (op->bufpos < op->buflen) {
        // There were more bytes left to transmit! 
        txn->flags = I2C_TXN_DONE | I2C_TXN_ERR;
        break; 
        //goto next_txn;
      }

      goto next_op;

    default:
      assert(0 && "unknown status in master transmitter mode");
    }
  }

  return;

next_op:

	/// TODO: REMOVE
	_delay_us(10);
	
  if (++(txn->opspos) < txn->opslen) {
    op = &txn->ops[txn->opspos];

    TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
    return;
  }

  txn->flags = I2C_TXN_DONE;
/*
next_txn:
  if (txn->next != NULL) {
    txn = txn->next;
    op = &txn->ops[0];

    TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTA);
    return;
  }
*/
  txn = NULL;
  op = NULL;

  TWCR = TWCR_DEFAULT | _BV(TWINT) | _BV(TWSTO);
}

/*
#define SCL_CLOCK  100000L

void __twi0_init__(void)
{
	hwtwi0_init(SCL_CLOCK); 
}

uint8_t __twi0_sync__(void){
	uint16_t timeout = 10000; 
	while(!(TWCR & (1<<TWINT)) && timeout) {
		_delay_us(1); 
		timeout--; 
	}
	return timeout != 0; 
}

uint8_t __twi0_waitStop__(void){
	uint16_t timeout = 100; 
	while((TWCR & (1<<TWSTO)) && timeout) {
		time_delay(1); 
		timeout--; 
	}
	return timeout != 0; 
}

unsigned char __twi0_start__(unsigned char address)
{
    uint8_t   twst;

	// send START condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);

	// wait until transmission completed (this is stupid!!)
	__twi0_sync__(); 

	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;

	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wail until transmission completed and ACK/NACK has been received
	__twi0_sync__(); 
	
	// check value of TWI Status Register. Mask prescaler bits.
	twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;

	return 0;

}

uint8_t __twi0_start_wait__(unsigned char address)
{
    uint8_t   twst;

    int retry = 2000; 
    while ( 1 )
    {
	    // send START condition
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
    
    	// wait until transmission completed
    	__twi0_sync__(); 
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    
    	// send device address
    	TWDR = address;
    	TWCR = (1<<TWINT) | (1<<TWEN);
    
    	// wail until transmission completed
    	__twi0_sync__(); 
    
    	// check value of TWI Status Register. Mask prescaler bits.
    	twst = TW_STATUS & 0xF8;
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ) 
    	{    	    
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	        
	        // wait until stop condition is executed and bus released
	        if(!__twi0_waitStop__()) continue; 

	        if(!(retry --)) break;  
    	    continue;
    	}
    	return 1; 
    	//if( twst != TW_MT_SLA_ACK) return 1;
    	break;
     }
	return 0; 
}

unsigned char __twi0_rep_start__(unsigned char address)
{
    return __twi0_start__( address );

}

void __twi0_stop__(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	
	// wait until stop condition is executed and bus released
	__twi0_waitStop__(); 

}

unsigned char __twi0_write__( unsigned char data )
{	
	uint8_t   twst;
    
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);

	// wait until transmission completed
	__twi0_sync__(); 

	// check value of TWI Status Register. Mask prescaler bits
	twst = TW_STATUS & 0xF8;
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;

}

unsigned char __twi0_readAck__(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	__twi0_sync__();    
	return TWDR;
}

unsigned char __twi0_readNak__(void)
{
	TWCR = (1<<TWINT) | (1<<TWEN);
	__twi0_sync__(); 
	return TWDR;
}*/
