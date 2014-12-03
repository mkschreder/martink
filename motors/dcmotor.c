/*
dcmotor lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <stdio.h>
#include <avr/io.h>
#include <util/delay.h>

#include "dcmotor.h"

/*
 * setup motor ports here
 */
void dcmotor_setup(void) {
	uint8_t i = 0;

	//motor 1
	i=0;
	dcmotor_array[i].ddr = &DDRD;
	dcmotor_array[i].port = &PORTD;
	dcmotor_array[i].pinA = PD2;
	dcmotor_array[i].pinB = PD3;

	//motor 2
	i=1;
	dcmotor_array[i].ddr = &DDRD;
	dcmotor_array[i].port = &PORTD;
	dcmotor_array[i].pinA = PD4;
	dcmotor_array[i].pinB = PD5;
}

/*
 * init a motor
 */
void dcmotor_init(void) {
	dcmotor_setup();

	//set ports
	uint8_t i = 0;
	for(i=0; i<DCMOTOR_MOTORNUM; i++) {
		*dcmotor_array[i].ddr |= (1 << dcmotor_array[i].pinA); //output
		*dcmotor_array[i].ddr |= (1 << dcmotor_array[i].pinB); //output
		*dcmotor_array[i].port &= ~(1 << dcmotor_array[i].pinA); //off
		*dcmotor_array[i].port &= ~(1 << dcmotor_array[i].pinB); //off
	}
}

/*
 * stop a motor
 */
void dcmotor_gostop(uint8_t motorid) {
	if(motorid >= 0 && motorid < DCMOTOR_MOTORNUM) {
		*dcmotor_array[motorid].port &= ~(1 << dcmotor_array[motorid].pinA);
		*dcmotor_array[motorid].port &= ~(1 << dcmotor_array[motorid].pinB);
	}
}

/*
 * let a motor go forward
 */
void dcmotor_goforward(uint8_t motorid) {
	if(motorid >= 0 && motorid < DCMOTOR_MOTORNUM) {
		*dcmotor_array[motorid].port &= ~(1 << dcmotor_array[motorid].pinA);
		*dcmotor_array[motorid].port |= (1 << dcmotor_array[motorid].pinB);
	}
}

/*
 * let a motor go backward
 */
void dcmotor_gobackward(uint8_t motorid) {
	if(motorid >= 0 && motorid < DCMOTOR_MOTORNUM) {
		*dcmotor_array[motorid].port |= (1 << dcmotor_array[motorid].pinA);
		*dcmotor_array[motorid].port &= ~(1 << dcmotor_array[motorid].pinB);
	}
}


