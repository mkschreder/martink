/*
fs300a water flow sensor lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  * http://www.seeedstudio.com/wiki/G3/4_Water_Flow_sensor
Notes:
  edit timer setting, to fit to your F_CPU

*/

#ifdef FS300A

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "fs300a.h"

volatile uint16_t fs300a_flowpulsecount = 0;
volatile uint16_t fs300a_timer0counter = 0;

/*
 * timer0 interrupt
 * here we count the pulse coming out from our fs300a hall sensor
 */
ISR(TIMER0_OVF_vect) {
	//we are here every TIMER0_MSREPEATMS
	static uint8_t input_state;
	static uint16_t tflowpulsecount;
	uint8_t i;
	uint8_t current_state;
	fs300a_timer0counter++;
	//every 1sec get the pulse for seconds, hertz
	if(fs300a_timer0counter>=TIMER0_FS300ARESET) {
		fs300a_timer0counter = 0;
		fs300a_flowpulsecount = tflowpulsecount/2; //get the rise edge only
		tflowpulsecount = 0;
	}
	current_state = (FS300A_PIN & (1<<FS300A_INPUT1))>>FS300A_INPUT1; //get current state
	i = input_state ^ ~current_state; //is it changed? (using xor)
	input_state = ~current_state; //set last input state as current state
	tflowpulsecount += i; //increment count if state is changed
}

/*
 * init fs300a
 */
void fs300a_init(void) {
	//set timer0
	TCCR0B = TIMER0_PRESCALER;
	TIMSK0 = 1<<TOIE0; // enable timer interrupt
	//input
	FS300A_DDR &= ~(1 << FS300A_INPUT1);
}

/*
 * get fs300a flow (litre\hour)
 */
uint16_t fs300a_getflow(void) {
	//(Pulse frequency x 60) / FS300A_CONVERSIONSENSITIVITY = flow rate
	return ((fs300a_flowpulsecount*60)/FS300A_CONVERSIONSENSITIVITY);
}

#endif
