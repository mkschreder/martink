/*
l74hc595 lib 0x01

copyright (c) Davide Gironi, 2011

References: bildr 74hc595 library for arduino
	http://bildr.org/2011/08/74hc595-breakout-arduino/

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "l74hc595.h"
#include <arch/soc.h>

#define L74HC595_STCLo {L74HC595_PORT &= ~_BV(L74HC595_STCPIN);}
#define L74HC595_STCHi {L74HC595_PORT |= _BV(L74HC595_STCPIN);}
/*
 * init the shift register
 */
void l74hc595_init(void) {
	spi_init(); 
	
	L74HC595_DDR |= _BV(L74HC595_STCPIN); 
	
	L74HC595_STCLo;  
}

void l74hc595_write(uint8_t data) {
	spi_writereadbyte(data); 
	L74HC595_STCHi; 
	_delay_us(1); // not needed but still for safety (16ns is minimum high period)
	L74HC595_STCLo; 
}
