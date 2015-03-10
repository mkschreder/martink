#ifndef _I2CMASTER_H
#define _I2CMASTER_H   1
/************************************************************************* 
* Title:    C include file for the I2C master interface 
*           (i2cmaster.S or twimaster.c)
* Author:   Peter Fleury <pfleury@gmx.ch>  http://jump.to/fleury
* File:     $Id: i2cmaster.h,v 1.10 2005/03/06 22:39:57 Peter Exp $
* Software: AVR-GCC 3.4.3 / avr-libc 1.2.3
* Target:   any AVR device
* Usage:    see Doxygen manual
**************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#if (__GNUC__ * 100 + __GNUC_MINOR__) < 304
#error "This library requires AVR-GCC 3.4 or later, update to newer AVR-GCC compiler !"
#endif

#include <avr/io.h>

#include "config.h"


struct _avr_twi_op {
  volatile uint8_t address;
  volatile uint8_t buflen;
  volatile uint8_t bufpos;
  volatile uint8_t *buf;
};

typedef struct _avr_twi_op avr_twi_op;

struct _avr_twi_device {
	volatile struct _avr_twi_op op; 
	//volatile uint8_t status;
};

extern volatile struct _avr_twi_device _twi0;
extern volatile uint8_t _twi0_status; 

#define hwtwi0_init(speed) ({\
	TWSR = 0;\
  TWBR = (uint8_t)(((F_CPU/speed)-16)/2);\
})

#define TWCR_DEFAULT (_BV(TWEN) | _BV(TWIE)) //_BV(TWEA)

//#define TWCR_NOT_ACK TWCR_DEFAULT | _BV(TWINT)
//#define TWCR_ACK (TWCR_NOT_ACK | _BV(TWEA))

#define TWI_ERR_BUSY 				0x0100
#define TWI_ERR_NO_RESPONSE 0x0200

/// always set when twi bus is ready to receive more data
#define TWI_READY 0
/// set when data has been sent but no stop condition has been sent yet
#define TWI_IDLE	1
#define TWI_NO_RESPONSE 2
#define TWI_REP_START_SENT 4

#include <kernel/dev/i2c.h>

int8_t i2cdev_init(uint8_t dev_id); 
void i2cdev_deinit(uint8_t dev_id); 
int8_t i2cdev_write(uint8_t dev_id, uint8_t addr, const uint8_t *data, uint8_t sz); 
int8_t i2cdev_read(uint8_t dev_id, uint8_t addr, uint8_t *data, uint8_t sz); 
int8_t i2cdev_stop(uint8_t dev_id); 
//void twi_wait(uint8_t dev_id, uint8_t addr); 
uint8_t i2cdev_status(uint8_t dev_id, i2cdev_status_t flags); 
uint8_t i2cdev_open(uint8_t dev_id); 
void i2cdev_close(uint8_t dev_id); 

/*
void twi0_slave_init(uint8_t addr);
uint8_t twi0_slave_data_available(void);
// read received data from buffer
void twi0_slave_read(uint8_t *data, uint8_t max_size);
// write response to a slave request or will fail if slave interface is not in response state
void twi0_slave_write(uint8_t *data, uint8_t data_size); 
*/

#ifdef __cplusplus
}
#endif

/**@}*/
#endif
