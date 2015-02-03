/*
	This file is part of martink project. 

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

	Github: https://github.com/mkschreder

	Contributors:
	* Davide Gironi - developing original driver
	* Martin K. Schröder - maintenance since Oct 2014
*/

#ifndef TSL235_H_
#define TSL235_H_

#include <stdio.h>

//time(ms) during which we count pulses
#define TSL235_COUNTTIMEMS 100
//because we cuont pulse in TSL235_RESETTIMEMS, and herz is 1(s)/freq, we need to convert the freq measured to made it 1s freq
//1000 / TSL235_RESETTIMEMS
#define TSL235_FREQMULTIPLIER 10

//to measure pulse we count the rising edge on external clock source pin
//so we have to define a timer interrupt to count rising edge
#define TIMER_INTERRUPTfunc \
	ISR(TIMER1_OVF_vect)
//a timer stop function
#define TIMER_STOP \
	TIMSK1 = 0; \
	TCCR1B = 0;
//a timer start function
#define TIMER_START \
	TIMSK1 = (1 << TOIE1); \
	TCCR1B = (1 << CS12) | (1 << CS11) | (1 << CS10); /*External clock source on T1 pin. Clock on rising edge*/
//the timer count register (that measure how many times the timer has run)
#define TIMER_COUNTREGISTER \
	TCNT1

//tsl235 constant to evaluate illuminance
//if we fix the lumisonisity function, the sensor responsivity, and the source spectral function
//we can evaluate a fixed constant to calculate the illuminance, the matlab helper provided will return this constant
#define TSL235_TK 1.9865

//full-scale frequency is the maximum operating frequency of the device without saturation
#define TSL235_FULLSCALE 500000

extern void tsl235_init(void);
extern uint32_t tsl235_getfreq(void);
extern int16_t tsl235_getilluminancebyfreq(uint32_t freq);
extern int16_t tsl235_getilluminance(void);


#endif
