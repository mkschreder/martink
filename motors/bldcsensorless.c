/*
bldc sensored driver 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef BLDCSENSORLESS

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "bldcsensorless.h"
#include BLDCSENSORLESS_ADCINCLUDE


//current direction
volatile uint8_t bldcsensorless_direction = BLDCSENSORLESS_DIRECTIONCW;
//current running status
volatile uint8_t bldcsensorless_enabled = 0;
//current speed
volatile uint16_t bldcsensorless_speed = 0;

//set admux tables
uint8_t bldcsensorless_admuxtableCW[] = BLDCSENSORLESS_ADMUXCW;
uint8_t bldcsensorless_admuxtableCCW[] = BLDCSENSORLESS_ADMUXCCW;

//current commutation step
uint8_t bldcsensorless_commutationstep = 0;


/*
 * get speed
 */
uint16_t bldcsensorless_getspeed(void) {
	return bldcsensorless_speed;
}


/*
 * set speed
 */
void bldcsensorless_setspeed(uint16_t speed) {
	cli();
	bldcsensorless_speed = (long)(speed - 0) * (long)(BLDCSENSORLESS_SPEEDMAX - BLDCSENSORLESS_SPEEDMIN) / (long)(100 - 0) + BLDCSENSORLESS_SPEEDMIN;
	ICR1 = bldcsensorless_speed;
	sei();
}


/*
 * get direction
 */
uint8_t bldcsensorless_getdirection(void) {
	return bldcsensorless_direction;
}


/*
 * set direction
 */
void bldcsensorless_setdirection(uint8_t direction) {
	if(direction == BLDCSENSORLESS_DIRECTIONCW || direction == BLDCSENSORLESS_DIRECTIONCCW) {
		cli();
		bldcsensorless_direction = direction;
		sei();
	}
}


/*
 * set stop
 */
void bldcsensorless_setstop(void) {
	if(bldcsensorless_enabled) {
		cli();
		BLDCSENSORLESS_RUNS;
		bldcsensorless_enabled = 0;
		sei();
	}
}


/*
 * set start
 */
void bldcsensorless_setstart(void) {
	if(!bldcsensorless_enabled) {
		cli();
		bldcsensorless_enabled = 1;
		sei();
	}
}


/*
 * do one sound cycle
 */
void bldcsensorless_soundercyle(uint16_t ontime, uint16_t offtime) {
	uint16_t i=0;
	for(i=0; i<ontime; i++)
		_delay_us(1);
	BLDCSENSORLESS_RUNB;
	BLDCSENSORLESS_RUNS;
	for(i=0; i<offtime; i++)
		_delay_us(1);
}


/*
 * emit a sound
 */
void bldcsensorless_sounder(uint8_t repetitions, uint8_t duration, uint16_t ontime, uint16_t offtime) {
	uint8_t i = 0;
	uint8_t q= 0;
	for (i=0; i<repetitions; i++)	 {
		for (q=0; q<duration; q++) {
			BLDCSENSORLESS_RUN1; bldcsensorless_soundercyle(ontime, offtime);
			BLDCSENSORLESS_RUN4; bldcsensorless_soundercyle(ontime, offtime);
			BLDCSENSORLESS_RUN3; bldcsensorless_soundercyle(ontime, offtime);
			BLDCSENSORLESS_RUN6; bldcsensorless_soundercyle(ontime, offtime);
			BLDCSENSORLESS_RUN5; bldcsensorless_soundercyle(ontime, offtime);
			BLDCSENSORLESS_RUN2; bldcsensorless_soundercyle(ontime, offtime);
		}
		_delay_ms(30);
		}
}


/*
 * emit the power on sound
 */
void bldcsensorless_poweronsound(void) {
	bldcsensorless_sounder(1, 20, 20, 1200);
	bldcsensorless_sounder(1, 20, 120, 1200);
	bldcsensorless_sounder(1, 20, 240, 1200);
}


/*
 * init bldc ports and variables
 */
void bldcsensorless_init(void) {
	cli();

	//init fet driver ports
	BLDCSENSORLESS_FETULDDR |= (1<<BLDCSENSORLESS_FETULINPUT);
	BLDCSENSORLESS_FETUHDDR |= (1<<BLDCSENSORLESS_FETUHINPUT);
	BLDCSENSORLESS_FETVLDDR |= (1<<BLDCSENSORLESS_FETVLINPUT);
	BLDCSENSORLESS_FETVHDDR |= (1<<BLDCSENSORLESS_FETVHINPUT);
	BLDCSENSORLESS_FETWLDDR |= (1<<BLDCSENSORLESS_FETWLINPUT);
	BLDCSENSORLESS_FETWHDDR |= (1<<BLDCSENSORLESS_FETWHINPUT);

	//set direction and speed
	bldcsensorless_setstop();
	bldcsensorless_setspeed(0);

	//init comparator port
	BLDCSENSORLESS_COMPDDR |= (1<<BLDCSENSORLESS_COMPINPUT); //output
	BLDCSENSORLESS_COMPPORT &= ~(1<<BLDCSENSORLESS_COMPINPUT); //off

	//precharge bootstrap_caps
	BLDCSENSORLESS_RUNB;
	_delay_ms(5);
	BLDCSENSORLESS_RUNS;

	ICR1 = TIMER1_ICR1; //ICR1
	TCCR1A = (1 << WGM11); //fast pwm mode
	TCCR1B = (1 << WGM13) | (1 << WGM12);
	TCCR1B |= TIMER1_PRESCALER; //prescaler
	TIMSK1 |= (1<<TOIE1); //enable timer1

	//sound start
	#if BLDCSENSORLESS_STARTUPSOUND == 1
	bldcsensorless_poweronsound();
	#endif

	sei();
}


/*
 * output a motor step
 */
void bldcsensorless_runstep(uint8_t step) {
	switch (step) {
		case 0:
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				BLDCSENSORLESS_101CW
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				BLDCSENSORLESS_101CCW
			}
			break;
		case 1:
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				BLDCSENSORLESS_100CW
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				BLDCSENSORLESS_100CCW
			}
			break;
		case 2:
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				BLDCSENSORLESS_110CW
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				BLDCSENSORLESS_110CCW
			}
			break;
		case 3:
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				BLDCSENSORLESS_010CW
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				BLDCSENSORLESS_010CCW
			}
			break;
		case 4:
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				BLDCSENSORLESS_011CW
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				BLDCSENSORLESS_011CCW
			}
			break;
		case 5:
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				BLDCSENSORLESS_001CW
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				BLDCSENSORLESS_001CCW
			}
			break;
	}

}


/*
 * startup motor cicle
 */
void bldcsensorless_startupmotor(void) {
	uint8_t i = 0;
	uint8_t j = 0;

	//setup startup delays array
	uint16_t startupdelays[BLDCSENSORLESS_STARTUPCOMMUTATIONS+1] = BLDCSENSORLESS_STARTUPDELAYS;

	//reset to first commutation
	bldcsensorless_commutationstep = 0;
	//run commutations
	for(;;) {
		bldcsensorless_runstep(bldcsensorless_commutationstep);
		if(bldcsensorless_commutationstep == 0) {
			if (i < 2)
				i += 1;
			if ((i == 2)&&(j<BLDCSENSORLESS_STARTUPCOMMUTATIONS)) {
				j += 1;i=0;
			}
			if ((i==2)&&(j==BLDCSENSORLESS_STARTUPCOMMUTATIONS)){
				break;
			}
		}
		//do startup delays
		uint16_t d = 0;
		for(d=0; d<startupdelays[j]; d++) {
			_delay_us(5);
		}
		//do next commutation
		bldcsensorless_commutationstep++;
		if(bldcsensorless_commutationstep > 5)
			bldcsensorless_commutationstep = 0;
	}
}


/*
 * main bldc timer
 */
ISR(TIMER1_OVF_vect) {
	static uint8_t emit = 1; //emit or skip emission
	static uint16_t zcerrors = 0; //count zc errors
	static uint16_t currentbemf = 0; //bemf value used in zc detection
	static uint8_t channel = 0; //zc channel selection
	static uint8_t startup = 0; //startup selector
	static uint8_t bldcsensorless_zcpolarity = 0; //zc polarity detection

	if(bldcsensorless_enabled) {

		//emission step
		if(emit) {
			BLDCSENSORLESS_RUNS;
			_delay_us(10);

			//run step
			bldcsensorless_runstep(bldcsensorless_commutationstep);

			//setup zc polarity for zc detection
			bldcsensorless_zcpolarity = bldcsensorless_commutationstep & 0x01;

			//setup adc channel for zc detection
			if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCW) {
				channel = bldcsensorless_admuxtableCW[bldcsensorless_commutationstep];
			} else if(bldcsensorless_direction == BLDCSENSORLESS_DIRECTIONCCW) {
				channel = bldcsensorless_admuxtableCCW[bldcsensorless_commutationstep];
			}
			BLDCSENSORLESS_ADCSETCHANNEL(channel);

			emit = 0; //do not emission step
			zcerrors = 0; //reset zc errors

			//do next commutation
			bldcsensorless_commutationstep++;
			if(bldcsensorless_commutationstep > 5)
				bldcsensorless_commutationstep = 0;
		}

		//not emission step (check zc)
		if(!emit) {
			//read actual bemf current
			currentbemf = BLDCSENSORLESS_ADCREADSEL;

			//check zc depending on actual polarity
			if ((!bldcsensorless_zcpolarity && currentbemf>BLDCSENSORLESS_ZCTHRESHOLD) || (bldcsensorless_zcpolarity && currentbemf<BLDCSENSORLESS_ZCTHRESHOLD)) {
				emit = 1; //do emission step

				BLDCSENSORLESS_RUNS;
			} else {
				//check for zc errors
				if(zcerrors < BLDCSENSORLESS_ZCERRORS) {
					zcerrors++;
				} else {
					zcerrors = 0; //reset zc errors
					startup = 1; //set startup
				}
			}
		}

		//startup step
		if(startup) {
			startup = 0; //reset startup

			//do startup motor function
			bldcsensorless_startupmotor();

			//reset to first commutation
			bldcsensorless_commutationstep = 0;

			emit = 1; //do emission step
		}
	}
}

#endif
