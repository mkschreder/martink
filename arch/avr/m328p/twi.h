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

#include "autoconf.h"
#include "../../twi.h"

#define hwtwi0_init(speed) ({\
	TWSR = 0;\
  TWBR = (uint8_t)(((F_CPU/speed)-16)/2);\
})
#define twi0_is_busy() PFCALL(CONFIG_TWI0_NAME, is_busy)
#define twi0_wait() ({ while(twi0_is_busy()); })
#define twi0_start_transaction(op_list) PFCALL(CONFIG_TWI0_NAME, start_transaction, op_list)

extern void PFDECL(CONFIG_TWI0_NAME, init, void);
extern uint8_t PFDECL(CONFIG_TWI0_NAME, is_busy, void);
extern void PFDECL(CONFIG_TWI0_NAME, start_transaction, i2c_op_list_t);

/*
extern void PFDECL(CONFIG_TWI0_NAME, stop, void);
extern unsigned char PFDECL(CONFIG_TWI0_NAME, start, unsigned char addr);
extern unsigned char PFDECL(CONFIG_TWI0_NAME, rep_start, unsigned char addr);
extern uint8_t PFDECL(CONFIG_TWI0_NAME, start_wait, unsigned char addr);
extern unsigned char PFDECL(CONFIG_TWI0_NAME, write, unsigned char data);
extern unsigned char PFDECL(CONFIG_TWI0_NAME, readAck, void);
extern unsigned char PFDECL(CONFIG_TWI0_NAME, readNak, void);
*/

#ifdef __cplusplus
}
#endif

/**@}*/
#endif
