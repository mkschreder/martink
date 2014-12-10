/*
dcmotorpwm lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <stdio.h>
#include <arch/soc.h>
#include "dcmotorpwm.h"

/*
 * init a motor
 */
void dcmotorpwm_init(void) {
	//set ports
	///TODO: make portable
	/*
	DCMOTORPWM_DDR |= (1<<DCMOTORPWM_PIN1);
	DCMOTORPWM_DDR |= (1<<DCMOTORPWM_PIN2);

	ICR1 = DCMOTORPWM_ICR1; //fixed top value

	TCCR1A |= (1<<COM1A1); //non-inverting mode for OC1A
	TCCR1A |= (1<<COM1B1); //non-inverting mode for OC1B

	TCCR1A |= (1<<WGM11); //fast pwm mode
	TCCR1B |= (1<<WGM13) | (1<<WGM12);

	TCCR1B |= DCMOTORPWM_PRESCALER; //set prescaler

	//stop motor
	OCR1A = 0;
	OCR1B = 0;*/
}

/*
 * stop the motor
 */
void dcmotorpwm_gostop(void) {
	//set orc
	///TODO: make portable
	/*OCR1A = 0;
	OCR1B = 0;*/
}

/*
 * let the motor go forward
 * set velocity from 0 to 100
 */
void dcmotorpwm_goforward(uint8_t vel) {
	//limit velocity
	///TODO: make portable
	if(vel > 100)
		vel = 100;
	//transform velocity to orc value
	uint16_t orctop = ((DCMOTORPWM_ICR1-DCMOTORPWM_MINVEL) * (vel / 100.0)) + DCMOTORPWM_MINVEL;
	//set orc
	/*OCR1A = orctop;
	OCR1B = 0;*/
}

/*
 * let a motor go backward
 * set velocity from 0 to 100
 */
void dcmotorpwm_gobackward(uint8_t vel) {
	//limit velocity
	if(vel > 100)
		vel = 100;
	//transform velocity to orc value
	uint16_t orctop = ((DCMOTORPWM_ICR1-DCMOTORPWM_MINVEL) * (vel / 100.0)) + DCMOTORPWM_MINVEL;
	//set orc
	/*OCR1A = 0;
	OCR1B = orctop;*/
}


