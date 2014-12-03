/*
tsl235 lib 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  + Arduino And The Taos TSL230R Light Sensor: Getting Started
    http://dronecolony.com/2008/11/13/arduino-and-the-taos-tsl230r-light-sensor-getting-started/
*/


#ifndef TSL235_H_
#define TSL235_H_

#include <stdio.h>
#include <avr/io.h>

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
