/*
tsl235 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef TSL235

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "tsl235.h"

volatile long tsl235_pulsecount = 0;

/*
 * timer interrupt
 */
TIMER_INTERRUPTfunc {
	//we are here on rising edge of clock source pin
	tsl235_pulsecount++;
}

/*
 * get frequency, count pulse for TSL235_PULSETIMEMS
 */
uint32_t tsl235_getfreq(void) {
	TIMER_START; //start timer
	_delay_ms(TSL235_COUNTTIMEMS); //delay for TSL235_COUNTTIMEMS
	TIMER_STOP; //stop timer
	uint32_t tsl235_freq = (TIMER_COUNTREGISTER + tsl235_pulsecount*65536)*TSL235_FREQMULTIPLIER; //calculate the frequency
	TIMER_COUNTREGISTER = 0; //clear timer count register
	tsl235_pulsecount = 0; //clear pulse counter
	return tsl235_freq;
}

/*
 * get illuminance (lux=lm/m^2)
 */
int16_t tsl235_getilluminancebyfreq(uint32_t freq) {
	if(freq > TSL235_FULLSCALE)
		return -1;
	else
		return ((double)freq/1000)*TSL235_TK;
}

/*
 * get illuminance (lux=lm/m^2), no parameters
 */
int16_t tsl235_getilluminance(void) {
	return tsl235_getilluminancebyfreq(tsl235_getfreq());
}

#endif
