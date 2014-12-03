/*
stepper02 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef STEPPER

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "stepper02.h"

//motor array
static stepper02m stepper02ma[STEPPER02_MOTORNUM];

//lookup array
uint8_t stepper02_motortypeunipolarhalf_lookup[] = STEPPER02_MOTORTYPEUNIPOLARHALF_LOOKUP;
uint8_t stepper02_motortypeunipolarfull_lookup[] = STEPPER02_MOTORTYPEUNIPOLARFULL_LOOKUP;
uint8_t stepper02_motortypebipolarhalf_lookup[] = STEPPER02_MOTORTYPEBIPOLARHALF_LOOKUP;
uint8_t stepper02_motortypebipolarfull_lookup[] = STEPPER02_MOTORTYPEBIPOLARFULL_LOOKUP;

/*
 * setup motor parameters here
 */
void stepper02_setup(void) {
	//setup arrays
	uint8_t stepper02_setuptype[] = {
			STEPPER02_MOTOR1TYPE,
			STEPPER02_MOTOR2TYPE,
			STEPPER02_MOTOR3TYPE
	};

	uint8_t stepper02_setupmotorpin1[] = {
			STEPPER02_MOTOR1PIN1,
			STEPPER02_MOTOR2PIN1,
			STEPPER02_MOTOR3PIN1
	};
	volatile uint8_t *stepper02_setupmotorport1[] = {
			&STEPPER02_MOTOR1PORT1,
			&STEPPER02_MOTOR2PORT1,
			&STEPPER02_MOTOR3PORT1,
	};
	volatile uint8_t *stepper02_setupmotorddr1[] = {
			&STEPPER02_MOTOR1DDR1,
			&STEPPER02_MOTOR2DDR1,
			&STEPPER02_MOTOR3DDR1
	};

	uint8_t stepper02_setupmotorpin2[] = {
			STEPPER02_MOTOR1PIN2,
			STEPPER02_MOTOR2PIN2,
			STEPPER02_MOTOR3PIN2
	};
	volatile uint8_t *stepper02_setupmotorport2[] = {
			&STEPPER02_MOTOR1PORT2,
			&STEPPER02_MOTOR2PORT2,
			&STEPPER02_MOTOR3PORT2
	};
	volatile uint8_t *stepper02_setupmotorddr2[] = {
			&STEPPER02_MOTOR1DDR2,
			&STEPPER02_MOTOR2DDR2,
			&STEPPER02_MOTOR3DDR2
	};

	uint8_t stepper02_setupmotorpin3[] = {
			STEPPER02_MOTOR1PIN3,
			STEPPER02_MOTOR2PIN3,
			STEPPER02_MOTOR3PIN3
	};
	volatile uint8_t *stepper02_setupmotorport3[] = {
			&STEPPER02_MOTOR1PORT3,
			&STEPPER02_MOTOR2PORT3,
			&STEPPER02_MOTOR3PORT3
	};
	volatile uint8_t *stepper02_setupmotorddr3[] = {
			&STEPPER02_MOTOR1DDR3,
			&STEPPER02_MOTOR2DDR3,
			&STEPPER02_MOTOR3DDR3
	};

	uint8_t stepper02_setupmotorpin4[] = {
			STEPPER02_MOTOR1PIN4,
			STEPPER02_MOTOR2PIN4,
			STEPPER02_MOTOR3PIN4
	};
	volatile uint8_t *stepper02_setupmotorport4[] = {
			&STEPPER02_MOTOR1PORT4,
			&STEPPER02_MOTOR2PORT4,
			&STEPPER02_MOTOR3PORT4,
	};
	volatile uint8_t *stepper02_setupmotorddr4[] = {
			&STEPPER02_MOTOR1DDR4,
			&STEPPER02_MOTOR2DDR4,
			&STEPPER02_MOTOR3DDR4
	};

	uint8_t motorid = 0;
	//setup a motor
	for(motorid = 0; motorid<STEPPER02_MOTORNUM; motorid++) {
		stepper02ma[motorid].steptot = 0;
		stepper02ma[motorid].direction = 0;
		stepper02ma[motorid].stepdelay = 0;
		stepper02ma[motorid].stepdelaytot = 0;
		if(stepper02_setuptype[motorid] == STEPPER02_MOTORTYPEUNIPOLARHALF) {
			stepper02ma[motorid].lookupstepnum = sizeof(stepper02_motortypeunipolarhalf_lookup);
			stepper02ma[motorid].lookup = stepper02_motortypeunipolarhalf_lookup;
		} else if(stepper02_setuptype[motorid] == STEPPER02_MOTORTYPEUNIPOLARFULL) {
			stepper02ma[motorid].lookupstepnum = sizeof(stepper02_motortypeunipolarfull_lookup);
			stepper02ma[motorid].lookup = stepper02_motortypeunipolarfull_lookup;
		} else if(stepper02_setuptype[motorid] == STEPPER02_MOTORTYPEBIPOLARHALF) {
			stepper02ma[motorid].lookupstepnum = sizeof(stepper02_motortypebipolarhalf_lookup);
			stepper02ma[motorid].lookup = stepper02_motortypebipolarhalf_lookup;
		} else if(stepper02_setuptype[motorid] == STEPPER02_MOTORTYPEBIPOLARFULL) {
			stepper02ma[motorid].lookupstepnum = sizeof(stepper02_motortypebipolarfull_lookup);
			stepper02ma[motorid].lookup = stepper02_motortypebipolarfull_lookup;
		}
		stepper02ma[motorid].pin1 = stepper02_setupmotorpin1[motorid];
		stepper02ma[motorid].port1 = stepper02_setupmotorport1[motorid];
		stepper02ma[motorid].ddr1 = stepper02_setupmotorddr1[motorid];
		stepper02ma[motorid].pin2 = stepper02_setupmotorpin2[motorid];
		stepper02ma[motorid].port2 = stepper02_setupmotorport2[motorid];
		stepper02ma[motorid].ddr2 = stepper02_setupmotorddr2[motorid];
		stepper02ma[motorid].pin3 = stepper02_setupmotorpin3[motorid];
		stepper02ma[motorid].port3 = stepper02_setupmotorport3[motorid];
		stepper02ma[motorid].ddr3 = stepper02_setupmotorddr3[motorid];
		stepper02ma[motorid].pin4 = stepper02_setupmotorpin4[motorid];
		stepper02ma[motorid].port4 = stepper02_setupmotorport4[motorid];
		stepper02ma[motorid].ddr4 = stepper02_setupmotorddr4[motorid];
	}
}


/*
 * set motor speed, is a multiple of TIMER0_USDELAY
 */
void stepper02_setsteptime(uint8_t motorid, uint16_t usdelay) {
	//set the step delay time in us
	if(usdelay > 0 && usdelay < TIMER0_USDELAY)
		stepper02ma[motorid].stepdelaytot = 1; //only 1 step
	else
		stepper02ma[motorid].stepdelaytot = (uint16_t)(usdelay/TIMER0_USDELAY);
}

/*
 * stop a motor
 */
void stepper02_gostop(uint8_t motorid) {
	stepper02ma[motorid].steptot = 0; //stop the go cycle
	*stepper02ma[motorid].port1 &= ~(1 << stepper02ma[motorid].pin1);
	*stepper02ma[motorid].port2 &= ~(1 << stepper02ma[motorid].pin2);
	*stepper02ma[motorid].port3 &= ~(1 << stepper02ma[motorid].pin3);
	*stepper02ma[motorid].port4 &= ~(1 << stepper02ma[motorid].pin4);
}

/*
 * let a motor go forward
 */
void stepper02_goforward(uint8_t motorid, uint16_t steps) {
	stepper02ma[motorid].direction = STEPPER02_DIRECTIONFORWARD;
	stepper02ma[motorid].steptot = steps;
}

/*
 * let a motor go backward
 */
void stepper02_gobackward(uint8_t motorid, uint16_t steps) {
	stepper02ma[motorid].direction = STEPPER02_DIRECTIONBACKWARD;
	stepper02ma[motorid].steptot = steps;
}

/*
 * get a motor step
 */
uint16_t stepper02_getstep(uint8_t motorid) {
	return stepper02ma[motorid].steptot;
}

/*
 * init a motor
 */
void stepper02_init(void) {
	uint8_t i = 0;

	//set timer0
	TCCR0B = TIMER0_PRESCALER;
	TIMSK0 = 1<<TOIE0; // enable timer interrupt

	//setup motors
	stepper02_setup();

	//init motors
	for(i=0; i<STEPPER02_MOTORNUM; i++) {
		*stepper02ma[i].ddr1 |= (1 << stepper02ma[i].pin1);
		*stepper02ma[i].ddr2 |= (1 << stepper02ma[i].pin2);
		*stepper02ma[i].ddr3 |= (1 << stepper02ma[i].pin3);
		*stepper02ma[i].ddr4 |= (1 << stepper02ma[i].pin4);
	}

	//init interrupt
	sei();
}

/*
 * check step for a motor
 * this functions is a definition because a for cycle slow down the timer interrupt
 */
#define stepper02_timerdostep(motorid) \
	/*check if we have step to run*/ \
	if(stepper02ma[motorid].steptot != 0) { \
		/*increment the stepdelay*/ \
		stepper02ma[motorid].stepdelay++; \
		/*if we are at a valid time*/ \
		if(stepper02ma[motorid].stepdelay == stepper02ma[motorid].stepdelaytot) { \
			/*do a motor step*/ \
			if (stepper02ma[motorid].direction == STEPPER02_DIRECTIONFORWARD) { \
				stepper02ma[motorid].lookupstep++; \
				if (stepper02ma[motorid].lookupstep == stepper02ma[motorid].lookupstepnum) \
					stepper02ma[motorid].lookupstep = 0; \
			} else if (stepper02ma[motorid].direction == STEPPER02_DIRECTIONBACKWARD){ \
				if (stepper02ma[motorid].lookupstep == 0) \
					stepper02ma[motorid].lookupstep = stepper02ma[motorid].lookupstepnum; \
				stepper02ma[motorid].lookupstep--; \
			} \
			uint8_t stepdata = stepper02ma[motorid].lookup[stepper02ma[motorid].lookupstep]; \
			/*put the motor step*/ \
			if(stepdata>>0 & 0b1) \
				*stepper02ma[motorid].port1 |= (1 << stepper02ma[motorid].pin1); \
			else \
				*stepper02ma[motorid].port1 &= ~(1 << stepper02ma[motorid].pin1); \
			if(stepdata>>1 & 0b1) \
				*stepper02ma[motorid].port2 |= (1 << stepper02ma[motorid].pin2); \
			else \
				*stepper02ma[motorid].port2 &= ~(1 << stepper02ma[motorid].pin2); \
			if(stepdata>>2 & 0b1) \
				*stepper02ma[motorid].port3 |= (1 << stepper02ma[motorid].pin3); \
			else \
				*stepper02ma[motorid].port3 &= ~(1 << stepper02ma[motorid].pin3); \
			if(stepdata>>3 & 0b1) \
				*stepper02ma[motorid].port4 |= (1 << stepper02ma[motorid].pin4); \
			else \
				*stepper02ma[motorid].port4 &= ~(1 << stepper02ma[motorid].pin4); \
			/*update the stepdelay*/ \
			stepper02ma[motorid].stepdelay = 0; \
			/*decrement step to run*/ \
			stepper02ma[motorid].steptot--; \
			/*check if the motor has to be stopped*/ \
			if(stepper02ma[motorid].steptot == 0) { \
				*stepper02ma[motorid].port1 &= ~(1 << stepper02ma[motorid].pin1); \
				*stepper02ma[motorid].port2 &= ~(1 << stepper02ma[motorid].pin2); \
				*stepper02ma[motorid].port3 &= ~(1 << stepper02ma[motorid].pin3); \
				*stepper02ma[motorid].port4 &= ~(1 << stepper02ma[motorid].pin4); \
			} \
		} \
	} \

/*
 * timer0 interrupt
 */
ISR(TIMER0_OVF_vect) {
	//we are here every TIMER0_USDELAY us

	//motor 1
	#if STEPPER02_MOTORNUM > 0
	stepper02_timerdostep(0);
	#endif
	//motor 2
	#if STEPPER02_MOTORNUM > 1
	stepper02_timerdostep(1);
	#endif
	//motor 3
	#if STEPPER02_MOTORNUM > 2
	stepper02_timerdostep(2);
	#endif

}

#endif
