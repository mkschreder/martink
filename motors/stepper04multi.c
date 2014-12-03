/*
stepper04multi lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef STEPPERMULTI

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "l74hc595.h"
#include "stepper04multi.h"

/*
 * motor array
 */
volatile static stepper04multim stepper04multima[STEPPER04MULTI_MOTORNUM];

/*
 * lookup array
 */
uint8_t stepper04multi_motortypeunipolarhalf_lookup[] = STEPPER04MULTI_MOTORTYPEUNIPOLARHALF_LOOKUP;
uint8_t stepper04multi_motortypeunipolarfull_lookup[] = STEPPER04MULTI_MOTORTYPEUNIPOLARFULL_LOOKUP;
uint8_t stepper04multi_motortypebipolarhalf_lookup[] = STEPPER04MULTI_MOTORTYPEBIPOLARHALF_LOOKUP;
uint8_t stepper04multi_motortypebipolarfull_lookup[] = STEPPER04MULTI_MOTORTYPEBIPOLARFULL_LOOKUP;

/*
 * setup motor parameters here
 */
void stepper04multi_setup(void) {
	//setup arrays
	uint8_t stepper04multi_setuptype[] = {
			STEPPER04MULTI_MOTOR01_TYPE,
			STEPPER04MULTI_MOTOR02_TYPE,
			STEPPER04MULTI_MOTOR03_TYPE,
			STEPPER04MULTI_MOTOR04_TYPE,
			STEPPER04MULTI_MOTOR05_TYPE,
			STEPPER04MULTI_MOTOR06_TYPE,
			STEPPER04MULTI_MOTOR07_TYPE,
			STEPPER04MULTI_MOTOR08_TYPE,
			STEPPER04MULTI_MOTOR09_TYPE,
			STEPPER04MULTI_MOTOR10_TYPE,
			STEPPER04MULTI_MOTOR11_TYPE,
			STEPPER04MULTI_MOTOR12_TYPE,
			STEPPER04MULTI_MOTOR13_TYPE,
			STEPPER04MULTI_MOTOR14_TYPE,
			STEPPER04MULTI_MOTOR15_TYPE
	};

	uint8_t l74hc595pinx = 0;
	uint8_t motorid = 0;
	//setup a motor
	for(motorid = 0; motorid<STEPPER04MULTI_MOTORNUM; motorid++) {
		stepper04multima[motorid].steptot = 0;
		stepper04multima[motorid].direction = 0;
		stepper04multima[motorid].stepdelay = 0;
		stepper04multima[motorid].stepdelaytot = 0;
		if(stepper04multi_setuptype[motorid] == STEPPER04MULTI_MOTORTYPEUNIPOLARHALF) {
			stepper04multima[motorid].lookupstepnum = sizeof(stepper04multi_motortypeunipolarhalf_lookup);
			stepper04multima[motorid].lookup = stepper04multi_motortypeunipolarhalf_lookup;
		} else if(stepper04multi_setuptype[motorid] == STEPPER04MULTI_MOTORTYPEUNIPOLARFULL) {
			stepper04multima[motorid].lookupstepnum = sizeof(stepper04multi_motortypeunipolarfull_lookup);
			stepper04multima[motorid].lookup = stepper04multi_motortypeunipolarfull_lookup;
		} else if(stepper04multi_setuptype[motorid] == STEPPER04MULTI_MOTORTYPEBIPOLARHALF) {
			stepper04multima[motorid].lookupstepnum = sizeof(stepper04multi_motortypebipolarhalf_lookup);
			stepper04multima[motorid].lookup = stepper04multi_motortypebipolarhalf_lookup;
		} else if(stepper04multi_setuptype[motorid] == STEPPER04MULTI_MOTORTYPEBIPOLARFULL) {
			stepper04multima[motorid].lookupstepnum = sizeof(stepper04multi_motortypebipolarfull_lookup);
			stepper04multima[motorid].lookup = stepper04multi_motortypebipolarfull_lookup;
		}
		stepper04multima[motorid].l74hc595pin1 = l74hc595pinx; l74hc595pinx++;
		stepper04multima[motorid].l74hc595pin2 = l74hc595pinx; l74hc595pinx++;
		stepper04multima[motorid].l74hc595pin3 = l74hc595pinx; l74hc595pinx++;
		stepper04multima[motorid].l74hc595pin4 = l74hc595pinx; l74hc595pinx++;
	}
}

/*
 * init a motor
 */
void stepper04multi_init(void) {
	//setup motors
	stepper04multi_setup();

	//set timer0
	TCCR0B = TIMER0_PRESCALER;
	TIMSK0 = 1<<TOIE0; // enable timer interrupt

	//init interrupt
	sei();

	//init l74hc595
	l74hc595_init();
}

/*
 * set motor speed, is a multiple of TIMER0_USDELAY
 */
void stepper04multi_setsteptime(uint8_t motorid, uint16_t usdelay) {
	if(usdelay > 0 && usdelay < TIMER0_USDELAY)
		stepper04multima[motorid].stepdelaytot = 1; //only 1 step
	else
		//set the step delay time in us
		stepper04multima[motorid].stepdelaytot = (uint16_t)(usdelay/TIMER0_USDELAY);
}

/*
 * stop a motor
 */
void stepper04multi_gostop(uint8_t motorid) {
	stepper04multima[motorid].steptot = 0; //stop the go cycle
	l74hc595_setreg(stepper04multima[motorid].l74hc595pin1, 0);
	l74hc595_setreg(stepper04multima[motorid].l74hc595pin2, 0);
	l74hc595_setreg(stepper04multima[motorid].l74hc595pin3, 0);
	l74hc595_setreg(stepper04multima[motorid].l74hc595pin4, 0);
}

/*
 * let a motor go forward
 */
void stepper04multi_goforward(uint8_t motorid, uint16_t steps) {
	stepper04multima[motorid].direction = STEPPER04MULTI_DIRECTIONFORWARD;
	stepper04multima[motorid].steptot = steps;
}

/*
 * let a motor go backward
 */
void stepper04multi_gobackward(uint8_t motorid, uint16_t steps) {
	stepper04multima[motorid].direction = STEPPER04MULTI_DIRECTIONBACKWARD;
	stepper04multima[motorid].steptot = steps;
}

/*
 * get a motor step
 */
uint16_t stepper04multi_getstep(uint8_t motorid) {
	return stepper04multima[motorid].steptot;
}

/*
 * set a stepper half byte
 * stepbyte is a 4 byte value
 * motorpin1 is the first pin to set
 */
void stepper04multi_l74hc595sethalfbyte(uint16_t motorpin1, uint8_t stepbyte) {
	uint16_t byteindex = (L74HC595_ICNUMBER-1)-motorpin1/8; //find the array byte to change
	uint8_t current = l74hc595_icarray[byteindex]; //get current array value
	uint8_t bytesel = motorpin1%8; //check if we are in the pin0 or pin4 of the l74hc595
	if(bytesel == 0) {
		current &= 0b00001111; //clean 4 bits
		current |= (stepbyte << 4); //set 4 bits of new value
		l74hc595_icarray[byteindex] = current; //current; //update new value to array
	} else if(bytesel == 4) {
		current &= 0b11110000; //clean 4 bits
		current |= (stepbyte << 0);//set 4 bits of new value
		l74hc595_icarray[byteindex] = current; //current; //update new value to array
	}
}

/*
 * check step for a motor
 * this functions is a definition because a for cycle slow down the timer interrupt
 */
#define stepper04multi_timerdostep(motorid) \
	/*check if we have step to run*/ \
	if(stepper04multima[motorid].steptot != 0) { \
		/*increment the stepdelay*/ \
		stepper04multima[motorid].stepdelay++; \
		/*if we are at a valid time*/ \
		if(stepper04multima[motorid].stepdelay == stepper04multima[motorid].stepdelaytot) { \
			/*do a motor step*/ \
			if (stepper04multima[motorid].direction == STEPPER04MULTI_DIRECTIONFORWARD) { \
				stepper04multima[motorid].lookupstep++; \
				if (stepper04multima[motorid].lookupstep == stepper04multima[motorid].lookupstepnum) \
					stepper04multima[motorid].lookupstep = 0; \
			} else if (stepper04multima[motorid].direction == STEPPER04MULTI_DIRECTIONBACKWARD) { \
				if (stepper04multima[motorid].lookupstep == 0) \
					stepper04multima[motorid].lookupstep = stepper04multima[motorid].lookupstepnum; \
				stepper04multima[motorid].lookupstep--; \
			} \
			/*put the motor step*/ \
			uint8_t stepdata = stepper04multima[motorid].lookup[stepper04multima[motorid].lookupstep]; \
			stepper04multi_l74hc595sethalfbyte(stepper04multima[motorid].l74hc595pin1, stepdata); \
			/*update the stepdelay*/ \
			stepper04multima[motorid].stepdelay = 0; \
			/*decrement step to run*/ \
			stepper04multima[motorid].steptot--; \
			/*check if the motor has to be stopped*/ \
			if(stepper04multima[motorid].steptot == 0) { \
				stepper04multi_l74hc595sethalfbyte(stepper04multima[motorid].l74hc595pin1, 0); \
			} \
		} \
	} \

/*
 * shift out a bit to 74hc595 chain
 */
#define stepper04multi_l74hc595fastshiftout(i, j) val = (l74hc595_icarray[i] & (1 << j))>>j; \
L74HC595_PORT &= ~(1 << L74HC595_SRCLKPIN); \
L74HC595_PORT |= (val << L74HC595_SERPIN); \
L74HC595_PORT |= (1 << L74HC595_SRCLKPIN); \
L74HC595_PORT &= ~(val << L74HC595_SERPIN);

/*
 * timer0 interrupt
 */
ISR(TIMER0_OVF_vect) {
	//we are here every TIMER0_USDELAY us

	//setup motors step
	#if STEPPER04MULTI_MOTORNUM > 0
	stepper04multi_timerdostep(0);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 1
	stepper04multi_timerdostep(1);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 2
	stepper04multi_timerdostep(2);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 3
	stepper04multi_timerdostep(3);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 4
	stepper04multi_timerdostep(4);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 5
	stepper04multi_timerdostep(5);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 6
	stepper04multi_timerdostep(6);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 7
	stepper04multi_timerdostep(7);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 8
	stepper04multi_timerdostep(8);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 9
	stepper04multi_timerdostep(9);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 10
	stepper04multi_timerdostep(10);
	#endif
	#if STEPPER04MULTI_MOTORNUM > 11
	stepper04multi_timerdostep(11);
	#endif

	//shift out motors step
	uint8_t val = 0;
	L74HC595_PORT &= ~(1 << L74HC595_RCLKPIN);
	#if L74HC595_ICNUMBER > 0
	stepper04multi_l74hc595fastshiftout(0, 0); stepper04multi_l74hc595fastshiftout(0, 1); stepper04multi_l74hc595fastshiftout(0, 2); stepper04multi_l74hc595fastshiftout(0, 3); stepper04multi_l74hc595fastshiftout(0, 4); stepper04multi_l74hc595fastshiftout(0, 5); stepper04multi_l74hc595fastshiftout(0, 6); stepper04multi_l74hc595fastshiftout(0, 7);
	#endif
	#if L74HC595_ICNUMBER > 1
	stepper04multi_l74hc595fastshiftout(1, 0); stepper04multi_l74hc595fastshiftout(1, 1); stepper04multi_l74hc595fastshiftout(1, 2); stepper04multi_l74hc595fastshiftout(1, 3); stepper04multi_l74hc595fastshiftout(1, 4); stepper04multi_l74hc595fastshiftout(1, 5); stepper04multi_l74hc595fastshiftout(1, 6); stepper04multi_l74hc595fastshiftout(1, 7);
	#endif
	#if L74HC595_ICNUMBER > 2
	stepper04multi_l74hc595fastshiftout(2, 0); stepper04multi_l74hc595fastshiftout(2, 1); stepper04multi_l74hc595fastshiftout(2, 2); stepper04multi_l74hc595fastshiftout(2, 3); stepper04multi_l74hc595fastshiftout(2, 4); stepper04multi_l74hc595fastshiftout(2, 5); stepper04multi_l74hc595fastshiftout(2, 6); stepper04multi_l74hc595fastshiftout(2, 7);
	#endif
	#if L74HC595_ICNUMBER > 3
	stepper04multi_l74hc595fastshiftout(3, 0); stepper04multi_l74hc595fastshiftout(3, 1); stepper04multi_l74hc595fastshiftout(3, 2); stepper04multi_l74hc595fastshiftout(3, 3); stepper04multi_l74hc595fastshiftout(3, 4); stepper04multi_l74hc595fastshiftout(3, 5); stepper04multi_l74hc595fastshiftout(3, 6); stepper04multi_l74hc595fastshiftout(3, 7);
	#endif
	#if L74HC595_ICNUMBER > 4
	stepper04multi_l74hc595fastshiftout(4, 0); stepper04multi_l74hc595fastshiftout(4, 1); stepper04multi_l74hc595fastshiftout(4, 2); stepper04multi_l74hc595fastshiftout(4, 3); stepper04multi_l74hc595fastshiftout(4, 4); stepper04multi_l74hc595fastshiftout(4, 5); stepper04multi_l74hc595fastshiftout(4, 6); stepper04multi_l74hc595fastshiftout(4, 7);
	#endif
	#if L74HC595_ICNUMBER > 5
	stepper04multi_l74hc595fastshiftout(5, 0); stepper04multi_l74hc595fastshiftout(5, 1); stepper04multi_l74hc595fastshiftout(5, 2); stepper04multi_l74hc595fastshiftout(5, 3); stepper04multi_l74hc595fastshiftout(5, 4); stepper04multi_l74hc595fastshiftout(5, 5); stepper04multi_l74hc595fastshiftout(5, 6); stepper04multi_l74hc595fastshiftout(5, 7);
	#endif
	L74HC595_PORT |= (1 << L74HC595_RCLKPIN);
}

#endif
