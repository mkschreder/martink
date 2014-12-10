/*
l74hc165 lib 0x01

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <string.h>
#include <arch/soc.h>

#include "l74hc165.h"

/*
 * init the shift register
 */
void l74hc165_init(void) {
	//output
	L74HC165_DDR |= (1 << L74HC165_CLOCKPIN);
	L74HC165_DDR |= (1 << L74HC165_LOADPIN);
	//input
	L74HC165_DDR &= ~(1 << L74HC165_DATAPIN);
	//low
	L74HC165_PORT &= ~(1 << L74HC165_CLOCKPIN);
	L74HC165_PORT &= ~(1 << L74HC165_LOADPIN);
}

/*
 * shift in data
 */
void l74hc165_shiftin(uint8_t *bytearray) {
	//parallel load to freeze the state of the data lines
	L74HC165_PORT &= ~(1 << L74HC165_LOADPIN);
	_delay_us(5);
	L74HC165_PORT |= (1 << L74HC165_LOADPIN);
	for(uint8_t i = 0; i < L74HC165_ICNUMBER; i++){
		//iterate through the bits in each registers
		uint8_t currentbyte = 0;
		for(uint8_t j = 0; j < 8; j++){
			currentbyte |= ((L74HC165_PIN & (1 << L74HC165_DATAPIN))>>L74HC165_DATAPIN)<<(7-j);
	        //get next
			L74HC165_PORT |= (1 << L74HC165_CLOCKPIN);
	        _delay_us(5);
	        L74HC165_PORT &= ~(1 << L74HC165_CLOCKPIN);
		}
		memcpy(&bytearray[i], &currentbyte, 1);
	}
}
