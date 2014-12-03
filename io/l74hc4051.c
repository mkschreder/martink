/*
l74hc4051 lib 0x01

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include <avr/io.h>

#include "l74hc4051.h"

/*
 * init the shift register
 */
void l74hc4051_init(void) {
	//output
	L74HC4051_DDR |= (1 << L74HC4051_S0PIN);
	L74HC4051_DDR |= (1 << L74HC4051_S1PIN);
	L74HC4051_DDR |= (1 << L74HC4051_S2PIN);
	//low
	L74HC4051_PORT &= ~(1 << L74HC4051_S0PIN);
	L74HC4051_PORT &= ~(1 << L74HC4051_S1PIN);
	L74HC4051_PORT &= ~(1 << L74HC4051_S2PIN);
}

/*
 * set a channel
 */
void l74hc4051_setchannel(uint8_t channel) {
	//bit 1
	if((channel & (1 << 0)) >> 0)
		L74HC4051_PORT |= (1 << L74HC4051_S0PIN);
	else
		L74HC4051_PORT &= ~(1 << L74HC4051_S0PIN);
	//bit 2
	if((channel & (1 << 1)) >> 1)
		L74HC4051_PORT |= (1 << L74HC4051_S1PIN);
	else
		L74HC4051_PORT &= ~(1 << L74HC4051_S1PIN);
	//bit 3
	if((channel & (1 << 2)) >> 2)
		L74HC4051_PORT |= (1 << L74HC4051_S2PIN);
	else
		L74HC4051_PORT &= ~(1 << L74HC4051_S2PIN);
}
