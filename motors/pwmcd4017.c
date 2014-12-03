/*
pwmcd4017 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef PWMCD4017

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "pwmcd4017.h"

#if PWMCD4017_HIGHACCU == 1

/*
 * return the number of clock to wait for the next timer interrupt
 * given a clock value
 * using this library the clock value is set according to PWMCD4017_FREQUS
 * but we must set it according to PWMCD4017_FREQUSREAL
 */
#define pwmcd4017_getnextclock(clock) (uint16_t)(65535 - ((uint16_t)clock * PWMCD4017_CLOCKDIFF) + 1)
static int16_t waitccyles = 0;
ISR(TIMER1_OVF_vect) {
	//we are here every FCPU/1+(prescaler)
	uint8_t pulse = 1;
	if(waitccyles > PWMCD4017_CLOCKMAX) {
		//we have a big time to wait, so lets split it many times
		pulse = 0; //do not pulse
		TCNT1 = pwmcd4017_getnextclock(PWMCD4017_CLOCKMAX); //preload timer for the next interrupt
		waitccyles = waitccyles-PWMCD4017_CLOCKMAX; //substract the time we have already wait
	} else {
		//do the last time count
		if(waitccyles != 0) {
			pulse = 0; //do not pulse
			TCNT1 = pwmcd4017_getnextclock(waitccyles); //preload timer for the next interrupt
			waitccyles = 0;
		}
	}
	if(pulse) {
		//go to next output
		pwmcd4017_array[0].currentpin++;
		if(pwmcd4017_array[0].currentpin == PWMCD4017_CHIPPIN)
			pwmcd4017_array[0].currentpin = 0;
		//read the number of cicle to wait
		waitccyles = pwmcd4017_array[0].clock[pwmcd4017_array[0].currentpin];
		//do pulse, set port high and low
		*pwmcd4017_array[0].clkport |= (1 << pwmcd4017_array[0].clkpin);
		*pwmcd4017_array[0].clkport &= ~(1 << pwmcd4017_array[0].clkpin);
		//get back immediatly to check the clock
		TCNT1 = 65535; //preload timer for the next interrupt
	}
}

#else

#define pwmcd4017_setclock(i) \
	pwmcd4017_array[i].clockcounter++; /*increment counter for current*/ \
	if(pwmcd4017_array[i].clockcounter > pwmcd4017_array[i].clock[pwmcd4017_array[i].currentpin]) { \
		/*do pulse, set port high and low*/ \
		*pwmcd4017_array[i].clkport |= (1 << pwmcd4017_array[i].clkpin); \
		*pwmcd4017_array[i].clkport &= ~(1 << pwmcd4017_array[i].clkpin); \
		/*reset counter*/ \
		pwmcd4017_array[i].clockcounter = 0; \
		/*go to next pin*/ \
		pwmcd4017_array[i].currentpin++; \
		if(pwmcd4017_array[i].currentpin == PWMCD4017_CHIPPIN) \
			pwmcd4017_array[i].currentpin = 0; \
	}

ISR(TIMER1_OVF_vect) {
	//here every 1/TIMER1_FREQ seconds

	//strange problem here, with a cycle it does not works well, so i've used the define directive
	//experiment here to see the max chip for the frequency you need
	uint8_t i = 0;

	//chip 1
	#if PWMCD4017_CHIP > 0
	pwmcd4017_setclock(i);
	#endif

	//chip 2
	#if PWMCD4017_CHIP > 1
	i++;
	pwmcd4017_setclock(i);
	#endif

	//chip 3
	#if PWMCD4017_CHIP > 2
	i++;
	pwmcd4017_setclock(i);
	#endif

	//chip 4
	#if PWMCD4017_CHIP > 3
	i++;
	pwmcd4017_setclock(i);
	#endif
}
#endif

/*
 * set a value
 */
void pwmcd4017_set(uint8_t id, uint16_t value) {
	uint8_t i = (uint8_t)(id / PWMCD4017_CHIPPIN);
	uint8_t n = (uint8_t)(id % PWMCD4017_CHIPPIN);
	if(i >= 0 && i < PWMCD4017_CHIP)
		pwmcd4017_array[i].clock[n] = value;
}

/*
 * init cd4017
 */
void pwmcd4017_init(void) {
	cli();

	//setup chip 1
	#if PWMCD4017_CHIP > 0
	memset(&pwmcd4017_array[0].clock, 0, sizeof(pwmcd4017_array[0].clock));
	pwmcd4017_array[0].clockcounter = 0;
	pwmcd4017_array[0].currentpin = 0;
	pwmcd4017_array[0].clkddr = &PWMCD4017_CLK0DDR;
	pwmcd4017_array[0].clkport = &PWMCD4017_CLK0PORT;
	pwmcd4017_array[0].clkpin = PWMCD4017_CLK0PIN;
	#endif

	//setup chip 2
	#if PWMCD4017_CHIP > 1
	memset(&pwmcd4017_array[1].clock, 0, sizeof(pwmcd4017_array[1].clock));
	pwmcd4017_array[1].clockcounter = 0;
	pwmcd4017_array[1].currentpin = 0;
	pwmcd4017_array[1].clkddr = &PWMCD4017_CLK1DDR;
	pwmcd4017_array[1].clkport = &PWMCD4017_CLK1PORT;
	pwmcd4017_array[1].clkpin = PWMCD4017_CLK1PIN;
	#endif

	//setup chip 3
	#if PWMCD4017_CHIP > 2
	memset(&pwmcd4017_array[2].clock, 0, sizeof(pwmcd4017_array[2].clock));
	pwmcd4017_array[2].clockcounter = 0;
	pwmcd4017_array[2].currentpin = 0;
	pwmcd4017_array[2].clkddr = &PWMCD4017_CLK2DDR;
	pwmcd4017_array[2].clkport = &PWMCD4017_CLK2PORT;
	pwmcd4017_array[2].clkpin = PWMCD4017_CLK2PIN;
	#endif

	//setup chip 4
	#if PWMCD4017_CHIP > 3
	memset(&pwmcd4017_array[3].clock, 0, sizeof(pwmcd4017_array[3].clock));
	pwmcd4017_array[3].clockcounter = 0;
	pwmcd4017_array[3].currentpin = 0;
	pwmcd4017_array[3].clkddr = &PWMCD4017_CLK3DDR;
	pwmcd4017_array[3].clkport = &PWMCD4017_CLK3PORT;
	pwmcd4017_array[3].clkpin = PWMCD4017_CLK3PIN;
	#endif

	//set clock ports
	uint8_t i = 0;
	for(i=0; i<PWMCD4017_CHIP; i++) {
		*pwmcd4017_array[i].clkddr |= (1 << pwmcd4017_array[i].clkpin); //output
		*pwmcd4017_array[i].clkport &= ~(1 << pwmcd4017_array[i].clkpin); //off
	}

	//set reset chip
	PWMCD4017_RSTDDR |= (1 << PWMCD4017_RSTPIN); //output
	PWMCD4017_RSTPORT &= ~(1 << PWMCD4017_RSTPIN); //off

	//do reset
	PWMCD4017_RSTPORT |= (1 << PWMCD4017_RSTPIN);
	PWMCD4017_RSTPORT &= ~(1 << PWMCD4017_RSTPIN);

	//set timer
	#if PWMCD4017_HIGHACCU == 1
	TCCR1A = 0; //normal mode
	TCCR1B = 0;
	TCCR1C = 0;
	TCCR1B |= TIMER1_PRESCALER;
	TIMSK1 |= (1<<TOIE1); //enable timer1
	#else
	ICR1 = TIMER1_ICR1; //ICR1
	TCCR1A = (1 << WGM11); //fast pwm mode
	TCCR1B = (1 << WGM13) | (1 << WGM12);
	TCCR1B |= TIMER1_PRESCALER; //prescaler
	TIMSK1 |= (1<<TOIE1); //enable timer1
	#endif

	sei();
}

#endif
