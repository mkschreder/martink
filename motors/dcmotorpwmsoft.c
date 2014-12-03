/*
dcmotorpwmsoft lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef DCMOTORSOFT
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "dcmotorpwmsoft.h"

/*
 * setup motor ports here
 */
void dcmotorpwmsoft_setup(void) {
	uint8_t i = 0;

	//motor 1
	i=0;
	dcmotorpwmsoft_array[i].ddr = &DDRD;
	dcmotorpwmsoft_array[i].port = &PORTD;
	dcmotorpwmsoft_array[i].pinA = PD2;
	dcmotorpwmsoft_array[i].pinB = PD3;

	//motor 2
	i=1;
	dcmotorpwmsoft_array[i].ddr = &DDRD;
	dcmotorpwmsoft_array[i].port = &PORTD;
	dcmotorpwmsoft_array[i].pinA = PD4;
	dcmotorpwmsoft_array[i].pinB = PD5;
}

/*
 * init a motor
 */
void dcmotorpwmsoft_init(void) {
	dcmotorpwmsoft_setup();

	//set ports
	uint8_t i = 0;
	for(i=0; i<DCMOTORPWMSOFT_MOTORNUM; i++) {
		*dcmotorpwmsoft_array[i].ddr |= (1 << dcmotorpwmsoft_array[i].pinA); //output
		*dcmotorpwmsoft_array[i].ddr |= (1 << dcmotorpwmsoft_array[i].pinB); //output
		*dcmotorpwmsoft_array[i].port &= ~(1 << dcmotorpwmsoft_array[i].pinA); //off
		*dcmotorpwmsoft_array[i].port &= ~(1 << dcmotorpwmsoft_array[i].pinB); //off
	}

	//init timer 0
	TCCR0B = TIMER0_PRESCALER;
	TIMSK0 = 1<<TOIE0; // enable timer interrupt

	sei();
}

//wehave to set this as a definition because a for within the timer overflow interrupt have problems
#define dcmotorpwmsoft_checkmotor(motorid) \
		if(dcmotorpwmsoft_array[motorid].step == dcmotorpwmsoft_array[0].vel) { \
			*dcmotorpwmsoft_array[motorid].port &= ~(1 << dcmotorpwmsoft_array[motorid].pinA); \
			*dcmotorpwmsoft_array[motorid].port &= ~(1 << dcmotorpwmsoft_array[motorid].pinB); \
		} else if(dcmotorpwmsoft_array[motorid].step == DCMOTORPWMSOFT_MAXVAL) { \
			dcmotorpwmsoft_array[motorid].step = 0; \
			if(dcmotorpwmsoft_array[motorid].direction == DCMOTORPWMSOFT_DIRECTIONFORWARD) { \
				*dcmotorpwmsoft_array[motorid].port |= (1 << dcmotorpwmsoft_array[motorid].pinA); \
				*dcmotorpwmsoft_array[motorid].port &= ~(1 << dcmotorpwmsoft_array[motorid].pinB); \
			} else if(dcmotorpwmsoft_array[motorid].direction == DCMOTORPWMSOFT_DIRECTIONBACKWARD) { \
				*dcmotorpwmsoft_array[motorid].port &= ~(1 << dcmotorpwmsoft_array[motorid].pinA); \
				*dcmotorpwmsoft_array[motorid].port |= (1 << dcmotorpwmsoft_array[motorid].pinB); \
			} \
		} \
		dcmotorpwmsoft_array[motorid].step++;

/*
 * timer0 interrupt
 */
ISR(TIMER0_OVF_vect) {
	//we are here every TIMER0_USDELAY us
	uint8_t motorid = 0;


	//motor 1
	#if DCMOTORPWMSOFT_MOTORNUM > 0
	dcmotorpwmsoft_checkmotor(motorid);
	#endif

	//motor 2
	motorid++;
	#if DCMOTORPWMSOFT_MOTORNUM > 1
	dcmotorpwmsoft_checkmotor(motorid);
	#endif

	//motor 3
	motorid++;
	#if DCMOTORPWMSOFT_MOTORNUM > 2
	dcmotorpwmsoft_checkmotor(motorid);
	#endif

	//motor 4
	motorid++;
	#if DCMOTORPWMSOFT_MOTORNUM > 3
	dcmotorpwmsoft_checkmotor(motorid);
	#endif
}

/*
 * stop a motor
 */
void dcmotorpwmsoft_gostop(uint8_t motorid) {
	if(motorid >= 0 && motorid < DCMOTORPWMSOFT_MOTORNUM) {
		dcmotorpwmsoft_array[motorid].direction = DCMOTORPWMSOFT_DIRECTIONSTOP;
		dcmotorpwmsoft_array[motorid].step = 0;
		dcmotorpwmsoft_array[motorid].vel = 0;
		*dcmotorpwmsoft_array[motorid].port &= ~(1 << dcmotorpwmsoft_array[motorid].pinA);
		*dcmotorpwmsoft_array[motorid].port &= ~(1 << dcmotorpwmsoft_array[motorid].pinB);
	}
}

/*
 * let a motor go forward
 */
void dcmotorpwmsoft_goforward(uint8_t motorid, uint16_t velocity) {
	if(motorid >= 0 && motorid < DCMOTORPWMSOFT_MOTORNUM) {
		dcmotorpwmsoft_array[motorid].direction = DCMOTORPWMSOFT_DIRECTIONFORWARD;
		dcmotorpwmsoft_array[motorid].step = 0;
		//limit velocity
		if(velocity > 100)
			velocity = 100;
		//transform velocity to orc value
		uint16_t vel = ((DCMOTORPWMSOFT_MAXVAL-DCMOTORPWMSOFT_MINVAL) * (velocity / 100.0)) + DCMOTORPWMSOFT_MINVAL + 1;
		dcmotorpwmsoft_array[motorid].vel = vel;
	}
}

/*
 * let a motor go backward
 */
void dcmotorpwmsoft_gobackward(uint8_t motorid, uint16_t velocity) {
	if(motorid >= 0 && motorid < DCMOTORPWMSOFT_MOTORNUM) {
		dcmotorpwmsoft_array[motorid].direction = DCMOTORPWMSOFT_DIRECTIONBACKWARD;
		dcmotorpwmsoft_array[motorid].step = 0;
		//limit velocity
		if(velocity > 100)
			velocity = 100;
		//transform velocity to orc value
		uint16_t vel = ((DCMOTORPWMSOFT_MAXVAL-DCMOTORPWMSOFT_MINVAL) * (velocity / 100.0)) + DCMOTORPWMSOFT_MINVAL + 1;
		dcmotorpwmsoft_array[motorid].vel = vel;
	}
}

#endif
