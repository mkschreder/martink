/*
sevseg lib 0x03

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <stdio.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "sevseg.h"

uint8_t sevseg_displays[] = SEVSEG_PINDISPA;

//active display
volatile uint8_t sevseg_displayactive = 0;

//lookup table ascii char to 7 segment
#define SEGN 0xFF
#define SEGU 0xF7 //unknown way of display this character
static const uint8_t PROGMEM sevsegascii_table[] = {
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	SEGN, SEGN, SEGN, SEGN,
	0xc0, 0xf9, 0xa4, 0xb0, // 0 1 2 3
	0x99, 0x92, 0x82, 0xf8, // 4 5 6 7
	0x80, 0x90, SEGN, SEGN, // 8 9
	SEGN, SEGN, SEGN, SEGN,
	SEGN, 0x88, 0x83, 0xc6, //   A B C
	0xa1, 0x86, 0x8e, 0xc2, // D E F G
	0x8b, 0xcf, 0xe1, SEGU, // H I J _
	0xc7, 0xc8, 0xab, 0xc0, // L M N O
	0x8c, 0x98, 0xaf, 0x92, // P Q R S
	0x87, 0xc1, 0xe3, SEGU, // T U V _
	SEGU, 0x91, 0xa4, SEGN, // _ Y Z
	SEGN, SEGN, SEGN, SEGN,
	SEGN, 0x88, 0x83, 0xc6, //   A B C
	0xa1, 0x86, 0x8e, 0xc2, // D E F G
	0x8b, 0xcf, 0xe1, SEGU, // H I J _
	0xc7, 0xc8, 0xab, 0xc0, // L M N O
	0x8c, 0x98, 0xaf, 0x92, // P Q R S
	0x87, 0xc1, 0xe3, SEGU, // T U V _
	SEGU, 0x91, 0xa4, SEGN, // _ Y Z
	SEGN, SEGN, SEGN, SEGN
};


/*
 * init the seven segment ports
 */
void sevseg_init(void) {
	//output
	SEVSEG_DDR |= (1 << SEVSEG_PINA);
	SEVSEG_DDR |= (1 << SEVSEG_PINB);
	SEVSEG_DDR |= (1 << SEVSEG_PINC);
	SEVSEG_DDR |= (1 << SEVSEG_PIND);
	SEVSEG_DDR |= (1 << SEVSEG_PINE);
	SEVSEG_DDR |= (1 << SEVSEG_PINF);
	SEVSEG_DDR |= (1 << SEVSEG_PING);
	SEVSEG_DDR |= (1 << SEVSEG_PINDOT);
	//off
	if(SEVSEG_TYPE == SEVSEG_TYPECC || SEVSEG_TYPE == SEVSEG_TYPECAT)
		SEVSEG_PORT = 0x00;
	else if(SEVSEG_TYPE == SEVSEG_TYPECA || SEVSEG_TYPE == SEVSEG_TYPECCT)
		SEVSEG_PORT = 0xFF;

	for(uint8_t i = 0; i<sizeof(sevseg_displays); i++) {
		SEVSEG_DDRDISP |= (1 << sevseg_displays[i]);
		if(SEVSEG_TYPE == SEVSEG_TYPECA || SEVSEG_TYPE == SEVSEG_TYPECAT)
			SEVSEG_PORTDISP |= (1 << sevseg_displays[i]);
		else if(SEVSEG_TYPE == SEVSEG_TYPECC  || SEVSEG_TYPE == SEVSEG_TYPECCT)
			SEVSEG_PORTDISP &= ~(1 << sevseg_displays[i]);
	}
}

/*
 * reset to first display active
 */
void sevseg_setfirstdisplayactive() {
	sevseg_displayactive = 0;
}

/*
 * print a character, and eventually the dot
 */
void sevseg_putc(uint8_t c, uint8_t dot) {
	uint8_t cdisp = pgm_read_byte(&sevsegascii_table[c]);
	if(dot)
		cdisp &= ~(1<<7);
	if(SEVSEG_TYPE == SEVSEG_TYPECC|| SEVSEG_TYPE == SEVSEG_TYPECAT)
		SEVSEG_PORT = 0XFF ^ cdisp;
	else if(SEVSEG_TYPE == SEVSEG_TYPECA || SEVSEG_TYPE == SEVSEG_TYPECCT)
		SEVSEG_PORT = cdisp;
	_delay_us(SEVSEG_USONTIME); //time for the led to show
}

/*
 * print an hex value
 */
void sevseg_puthex(uint8_t hex) {
	SEVSEG_PORT = hex;
	_delay_us(SEVSEG_USONTIME); //time for the led to show
}

/*
 * select which display to run, continuous iteration to next display
 */
uint8_t sevseg_selnextdisplay(void) {
	//off digit
	if(SEVSEG_TYPE == SEVSEG_TYPECC || SEVSEG_TYPE == SEVSEG_TYPECAT)
		SEVSEG_PORT = 0X00;
	else if(SEVSEG_TYPE == SEVSEG_TYPECA || SEVSEG_TYPE == SEVSEG_TYPECCT)
		SEVSEG_PORT = 0xFF;
	//off display
	if(SEVSEG_TYPE == SEVSEG_TYPECA || SEVSEG_TYPE == SEVSEG_TYPECAT) {
		SEVSEG_PORTDISP |= (1 << sevseg_displays[sevseg_displayactive]);
	} else if(SEVSEG_TYPE == SEVSEG_TYPECC || SEVSEG_TYPE == SEVSEG_TYPECCT) {
		SEVSEG_PORTDISP &= ~(1 << sevseg_displays[sevseg_displayactive]);
	}
	if(sevseg_displayactive < sizeof(sevseg_displays)-1)
		sevseg_displayactive++;
	else
		sevseg_displayactive = 0;
	 //on display
	if(SEVSEG_TYPE == SEVSEG_TYPECA || SEVSEG_TYPE == SEVSEG_TYPECAT) {
		SEVSEG_PORTDISP &= ~(1 << sevseg_displays[sevseg_displayactive]);
	} else if(SEVSEG_TYPE == SEVSEG_TYPECC || SEVSEG_TYPE == SEVSEG_TYPECCT) {
		SEVSEG_PORTDISP |= (1 << sevseg_displays[sevseg_displayactive]);
	}
	return sevseg_displayactive;
}
