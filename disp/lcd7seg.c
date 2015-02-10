/**
	7 segment led display driver for 8 bit parallel port

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder

	Special thanks to:
	* Davide Gironi, original implementation
*/


#include <stdio.h>
#include "lcd7seg.h"

#include <arch/soc.h>

#define SEVSEG_PINA 0
#define SEVSEG_PINB 1
#define SEVSEG_PINC 2
#define SEVSEG_PIND 3
#define SEVSEG_PINE 4
#define SEVSEG_PINF 5
#define SEVSEG_PING 6
#define SEVSEG_PINDOT 7

//set minimum power on display time (us)
#define SEVSEG_USONTIME 1000

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

#define set_data(data) (pio_write_word(self->port, self->data_port, data))
#define set_cs_pin() (pio_write_pin(self->port, self->cs_pin, 1))
#define clear_cs_pin() (pio_write_pin(self->port, self->cs_pin, 0))

void led7seg_off(struct led7seg *self){
	if(self->type == SEVSEG_TYPECC || self->type == SEVSEG_TYPECAT)
		set_data(0x00); 
	else if(self->type == SEVSEG_TYPECA || self->type == SEVSEG_TYPECCT)
		set_data(0xFF);
		
	if(self->type == SEVSEG_TYPECA || self->type == SEVSEG_TYPECAT)
		set_cs_pin(); 
	else if(self->type == SEVSEG_TYPECC  || self->type == SEVSEG_TYPECCT)
		clear_cs_pin();
}
 
void led7seg_on(struct led7seg *self){
	set_data(self->data);
	
	if(self->type == SEVSEG_TYPECA || self->type == SEVSEG_TYPECAT)
		set_cs_pin(); 
	else if(self->type == SEVSEG_TYPECC  || self->type == SEVSEG_TYPECCT)
		clear_cs_pin();
}
 
void led7seg_init(struct led7seg *self, pio_dev_t port, uint8_t data_port, gpio_pin_t cs_pin, lcd7seg_type type) {
	self->port = port;
	self->data_port = data_port;
	self->cs_pin = cs_pin;
	self->type = type;
	
	//off
	
	if(self->type == SEVSEG_TYPECC || self->type == SEVSEG_TYPECAT)
		self->data = 0x00; 
	else if(self->type == SEVSEG_TYPECA || self->type == SEVSEG_TYPECCT)
		self->data = 0xFF; 

	led7seg_off(self); 
}
/*
void sevseg_putc(struct led7seg *self, uint8_t c, uint8_t dot) {
	uint8_t cdisp = pgm_read_byte(&sevsegascii_table[c]);
	if(dot)
		cdisp &= ~(1<<7);
	if(self->type == SEVSEG_TYPECC|| self->type == SEVSEG_TYPECAT)
		self->data = 0XFF ^ cdisp; 
	else if(self->type == SEVSEG_TYPECA || self->type == SEVSEG_TYPECCT)
		self->data = cdisp;
}*/
