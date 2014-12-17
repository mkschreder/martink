/*
l74hc595 lib 0x01

copyright (c) Davide Gironi, 2011

References: bildr 74hc595 library for arduino
	http://bildr.org/2011/08/74hc595-breakout-arduino/

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>

#include <arch/soc.h>

#include "l74hc595.h"

#ifndef CONFIG_L74HC595_STC_PIN
#define CONFIG_L74HC595_STC_PIN GPIO_NONE
#endif

#undef spi_init
#undef spi_writereadbyte

#define spi_init() PFCALL(CONFIG_SPI0_NAME, init)
#define spi_writereadbyte(b) (__spi0_putc__(0, b), __spi0_getc__(0))
/*
#define L74HC595_STCLo {L74HC595_PORT &= ~_BV(L74HC595_STCPIN);}
#define L74HC595_STCHi {L74HC595_PORT |= _BV(L74HC595_STCPIN);}
*/
#define L74HC595_STCLo gpio_write_pin(CONFIG_L74HC595_STC_PIN, 0)
#define L74HC595_STCHi gpio_write_pin(CONFIG_L74HC595_STC_PIN, 1)
/*
 * init the shift register
 */
void l74hc595_init(void) {
	spi_init(); 

	gpio_configure(CONFIG_L74HC595_STC_PIN, GP_OUTPUT);
	
	L74HC595_STCLo;  
}

void l74hc595_write(uint8_t data) {
	spi_writereadbyte(data); 
	L74HC595_STCHi; 
	delay_us(1); // not needed but still for safety (16ns is minimum high period)
	L74HC595_STCLo; 
}
