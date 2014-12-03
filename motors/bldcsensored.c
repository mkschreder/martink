/*
bldc sensored driver 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifdef BLDCSENSORED

#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "bldcsensored.h"

//current direction
volatile uint8_t bldcsensored_direction = BLDCSENSORED_DIRECTIONCW;
//current speed
volatile uint16_t bldcsensored_speed = 0;
//current running status
volatile uint8_t bldcsensored_enabled = 0;
//current spinning direction
volatile uint8_t bldcsensored_spindirection = BLDCSENSORED_SPINCW;

#if BLDCSENSORED_DEBUG == 1
//motor position debug variables
volatile uint8_t bldcsensored_debugstate = 0;
volatile uint8_t bldcsensored_debugposerror = 0;
volatile uint8_t bldcsensored_debugposerrorprev = 0;
#endif


/*
 * get current spin direction
 */
uint8_t bldcsensored_getspindirection(void) {
	return bldcsensored_spindirection;
}

/*
 * get speed
 */
uint16_t bldcsensored_getspeed(void) {
	return bldcsensored_speed;
}

/*
 * set speed
 */
void bldcsensored_setspeed(uint16_t speed) {
	cli();
	bldcsensored_speed = (long)BLDCSENSORED_SPEEDMIN - ((long)(speed - 0) * (long)(BLDCSENSORED_SPEEDMIN - 0) / (long)(100 - 0) + 0);
	sei();
}

/*
 * get direction
 */
uint8_t bldcsensored_getdirection(void) {
	return bldcsensored_direction;
}

/*
 * set direction
 */
void bldcsensored_setdirection(uint8_t direction) {
	if(direction == BLDCSENSORED_DIRECTIONCW || direction == BLDCSENSORED_DIRECTIONCCW) {
		cli();
		bldcsensored_direction = direction;
		sei();
	}
}

/*
 * set stop
 */
void bldcsensored_setstop(void) {
	if(bldcsensored_enabled) {
		cli();
		BLDCSENSORED_RUNS;
		bldcsensored_enabled = 0;
		sei();
	}
}

/*
 * set start
 */
void bldcsensored_setstart(void) {
	if(!bldcsensored_enabled) {
		cli();

		bldcsensored_enabled = 1;
		sei();
	}
}

/*
 * do one sound cycle
 */
void bldcsensored_soundercyle(uint16_t ontime, uint16_t offtime) {
	uint16_t i=0;
	for(i=0; i<ontime; i++)
		_delay_us(1);
	BLDCSENSORED_RUNB;
	BLDCSENSORED_RUNS;
	for(i=0; i<offtime; i++)
		_delay_us(1);
}

/*
 * emit a sound
 */
void bldcsensored_sounder(uint8_t repetitions, uint8_t duration, uint16_t ontime, uint16_t offtime) {
	uint8_t i = 0;
	uint8_t q= 0;
	for (i=0; i<repetitions; i++)	 {
		for (q=0; q<duration; q++) {
			BLDCSENSORED_RUN1; bldcsensored_soundercyle(ontime, offtime);
			BLDCSENSORED_RUN4; bldcsensored_soundercyle(ontime, offtime);
			BLDCSENSORED_RUN3; bldcsensored_soundercyle(ontime, offtime);
			BLDCSENSORED_RUN6; bldcsensored_soundercyle(ontime, offtime);
			BLDCSENSORED_RUN5; bldcsensored_soundercyle(ontime, offtime);
			BLDCSENSORED_RUN2; bldcsensored_soundercyle(ontime, offtime);
		}
		_delay_ms(30);
		}
}

/*
 * emit the power on sound
 */
void bldcsensored_poweronsound(void) {
	bldcsensored_sounder(1, 20, 20, 1200);
	bldcsensored_sounder(1, 20, 120, 1200);
	bldcsensored_sounder(1, 20, 240, 1200);
}

/*
 * init bldc ports and variables
 */
void bldcsensored_init(void) {
	cli();

	//init hall sensor ports
	BLDCSENSORED_HALLUDDR &= ~(1<<BLDCSENSORED_HALLUINPUT);
	BLDCSENSORED_HALLVDDR &= ~(1<<BLDCSENSORED_HALLVINPUT);
	BLDCSENSORED_HALLWDDR &= ~(1<<BLDCSENSORED_HALLWINPUT);

	//init fet driver ports
	BLDCSENSORED_FETULDDR |= (1<<BLDCSENSORED_FETULINPUT);
	BLDCSENSORED_FETUHDDR |= (1<<BLDCSENSORED_FETUHINPUT);
	BLDCSENSORED_FETVLDDR |= (1<<BLDCSENSORED_FETVLINPUT);
	BLDCSENSORED_FETVHDDR |= (1<<BLDCSENSORED_FETVHINPUT);
	BLDCSENSORED_FETWLDDR |= (1<<BLDCSENSORED_FETWLINPUT);
	BLDCSENSORED_FETWHDDR |= (1<<BLDCSENSORED_FETWHINPUT);

	//set direction and speed
	bldcsensored_setstop();
	bldcsensored_setspeed(0);

	//precharge bootstrap_caps
	BLDCSENSORED_RUNB;
	_delay_ms(5);
	BLDCSENSORED_RUNS;

	//sound start
	#if BLDCSENSORED_STARTUPSOUND == 1
	bldcsensored_poweronsound();
	#endif

	ICR1 = TIMER1_ICR1; //ICR1
	TCCR1A = (1 << WGM11); //fast pwm mode
	TCCR1B = (1 << WGM13) | (1 << WGM12);
	TCCR1B |= TIMER1_PRESCALER; //prescaler
	TIMSK1 |= (1<<TOIE1); //enable timer1

	sei();
}

/*
 * main bldc motor timer
 */
ISR(TIMER1_OVF_vect) {
	static uint8_t statenum = 0; //current motor position
	static uint8_t statenumprev = 1; //previous motor position (used to check spinning and direction), must be init from 1 to 6
	static uint16_t emitcounter = 0; //counter for the PWM emission
	static uint8_t emit = 0; //PWM controller
	static uint8_t emitdo = 0; //output to fet controller
	static uint8_t emitstartup = 0; //emit startup sequence
	static uint8_t hallstatus = 0; //current hall status
	static uint8_t hallstatuslast = 0; //last hall status
	static uint8_t wrongspin = 0; //wrong spin detector
	static uint8_t emitwrongspin = 0; //emit a wrong status, i.e. break motor
	static uint16_t stopcount = 0; //counter for the stop position
	static uint16_t spind = 0; //current spin direction
	static uint16_t spindlast = 0; //last spin direction counter
	static uint16_t spindlastcount = 0; //counter for the spin direction

	//speed controller
	if(emitcounter < bldcsensored_speed) {
		emitcounter++; //increase speed counter
	} else {
		emitcounter = 0;
		if(bldcsensored_enabled) { //emit only if enabled
			emit = 1; //this clock emit output to fet
			if(wrongspin) { //emit output error if we are not spinning the right way
				//update current spin direction
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) {
					spind = BLDCSENSORED_SPINCCW;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) {
					spind = BLDCSENSORED_SPINCW;
				}
				wrongspin = 0; //reset wrong spin counter
				emitwrongspin = 1; //output wrong status
			} else {
				//update current spin direction
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) {
					spind = BLDCSENSORED_SPINCW;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) {
					spind = BLDCSENSORED_SPINCCW;
				}
			}
		}
	}

	if(stopcount > BLDCSENSORED_STOPSTEPS) { //check if the motor is not spinning
		stopcount = 0; //reset stop position counter

		if(bldcsensored_enabled) { //emit only if enabled
			//if is not spinning emit a startup status
			emitdo = 1;
			emitstartup = 1;
		}

		spind = BLDCSENSORED_SPINNONE;
	}

	//update spin direction
	if(spindlast != spind) {
		spindlastcount++;
		if(spindlastcount>200) { //update direction if we are spinning many times this way
			spindlastcount = 0;
			spindlast = spind;
			bldcsensored_spindirection = spind;
		}
	}

	//get current hall status
	hallstatus = (
			(!((BLDCSENSORED_HALLUPIN & (1<<BLDCSENSORED_HALLUINPUT))>>BLDCSENSORED_HALLUINPUT)<<2) |
			(!((BLDCSENSORED_HALLVPIN & (1<<BLDCSENSORED_HALLVINPUT))>>BLDCSENSORED_HALLVINPUT)<<1) |
			(!((BLDCSENSORED_HALLWPIN & (1<<BLDCSENSORED_HALLWINPUT))>>BLDCSENSORED_HALLWINPUT)<<0)
			) & 0b00000111;

	//check if motor is in a different position
	if(hallstatus != hallstatuslast && !emitstartup) {
		hallstatuslast = hallstatus;
		stopcount = 0; //reset stop position counter

		uint8_t statprevok = 0; //check if previous position is right

		//free spin
		BLDCSENSORED_RUNS;

		switch (hallstatus) {
			case 0b00000101:
				statenum = 1;
				#if BLDCSENSORED_DEBUG == 1
				bldcsensored_debugstate = 101;
				#endif
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW && statenumprev == 6) { //check previous motor position dependence on direction
					statprevok = 1;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW && statenumprev == 2) { //direction CCW
					statprevok = 1;
				}
				break;
			case 0b00000100:
				statenum = 2;
				#if BLDCSENSORED_DEBUG == 1
				bldcsensored_debugstate = 100;
				#endif
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW && statenumprev == 1) { //direction CW
					statprevok = 1;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW && statenumprev == 3) { //direction CCW
					statprevok = 1;
				}
				break;
			case 0b00000110:
				statenum = 3;
				#if BLDCSENSORED_DEBUG == 1
				bldcsensored_debugstate = 110;
				#endif
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW && statenumprev == 2) { //direction CW
					statprevok = 1;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW && statenumprev == 4) { //direction CCW
					statprevok = 1;
				}
				break;
			case 0b00000010:
				statenum = 4;
				#if BLDCSENSORED_DEBUG == 1
				bldcsensored_debugstate = 10;
				#endif
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW && statenumprev == 3) { //direction CW
					statprevok = 1;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW && statenumprev == 5) { //direction CCW
					statprevok = 1;
				}
				break;
			case 0b00000011:
				statenum = 5;
				#if BLDCSENSORED_DEBUG == 1
				bldcsensored_debugstate = 11;
				#endif
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW && statenumprev == 4) { //direction CW
					statprevok = 1;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW && statenumprev == 6) { //direction CCW
					statprevok = 1;
				}
				break;
			case 0b00000001:
				statenum = 6;
				#if BLDCSENSORED_DEBUG == 1
				bldcsensored_debugstate = 1;
				#endif
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW && statenumprev == 5) { //direction CW
					statprevok = 1;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW && statenumprev == 1) { //direction CCW
					statprevok = 1;
				}
				break;
		}

		statenumprev = statenum; //update last state

		//check position for emissions
		if(statprevok == 1) { //previous position is ok
			//output to fet if spee counter permit emission
			if(emit) {
				emit = 0;
				emitdo = 1;
			}
		} else {
			wrongspin=1; //wrong spin detector
			#if BLDCSENSORED_DEBUG == 1
			if(bldcsensored_debugposerror == 0) {
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) {
					bldcsensored_debugposerror = 10 + statenum;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) {
					bldcsensored_debugposerror = 100 + statenum;
				}
				bldcsensored_debugposerrorprev = statenumprev;
			}
			#endif
		}

	} else {
		//hall sensor in the previos position as before
		if(stopcount <= BLDCSENSORED_STOPSTEPS)
			stopcount++; //increment stop position counter
	}

	//output fet status
	if(emitdo) {
		emitdo = 0;

		if(emitstartup) {
			emitstartup = 0; //reset emit startup

			uint16_t startup_delays[BLDCSENSORED_STARTUPCOMMUTATIONS+1] = BLDCSENSORED_STARTUPDELAYS; //startup delays

			//do a complete startup loop starting from the current motorpositon
			uint8_t i = 0;
			uint8_t go = statenum;

			while(i<BLDCSENSORED_STARTUPCOMMUTATIONS) {
				BLDCSENSORED_RUNS;
				_delay_us(15);

				switch (go) {
					case 1:
						statenumprev = 1; //update last motor position
						if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
							BLDCSENSORED_101CW;
						} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
							BLDCSENSORED_101CCW;
						}
						break;
					case 2:
						statenumprev = 2; //update last motor position
						if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
							BLDCSENSORED_100CW;
						} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
							BLDCSENSORED_100CCW;
						}
						break;
					case 3:
						statenumprev = 3; //update last motor position
						if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
							BLDCSENSORED_110CW;
						} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
							BLDCSENSORED_110CCW;
						}
						break;
					case 4:
						statenumprev = 4; //update last motor position
						if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
							BLDCSENSORED_010CW;
						} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
							BLDCSENSORED_010CCW;
						}
						break;
					case 5:
						statenumprev = 5; //update last motor position
						if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
							BLDCSENSORED_011CW;
						} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
							BLDCSENSORED_011CCW;
						}
						break;
					case 6:
						statenumprev = 6; //update last motor position
						if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
							BLDCSENSORED_RUN3;
						} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
							BLDCSENSORED_001CCW;
						}
						break;
				}

				//check startup direction
				if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
					go++;
				} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
					go--;
				}

				if(go > 6)
					go = go-6;
				else if(go < 1)
					go = 6;

				uint16_t m = 0;
				//do delay for startup
				for(m=0; m<startup_delays[i]; m++) {
					_delay_us(BLDCSENSORED_STARTUPDELAYMULT);
				}

				i++;
			}

			hallstatuslast = 0; //reset last status

		} else if(emitwrongspin) {
			BLDCSENSORED_RUNS; //wrong spin, emit break or spin
			emitwrongspin = 0;
		} else {
			//output depending on direction and position
			switch (hallstatus) {
				case 0b00000101:
					if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
						BLDCSENSORED_101CW;
					} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
						BLDCSENSORED_101CCW;
					}
					break;
				case 0b00000100:
					if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
						BLDCSENSORED_100CW;
					} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
						BLDCSENSORED_100CCW;
					}
					break;
				case 0b00000110:
					if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
						BLDCSENSORED_110CW;
					} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
						BLDCSENSORED_110CCW;
					}
					break;
				case 0b00000010:
					if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
						BLDCSENSORED_010CW;
					} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
						BLDCSENSORED_010CCW;
					}
					break;
				case 0b00000011:
					if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
						BLDCSENSORED_011CW;
					} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
						BLDCSENSORED_011CCW;
					}
					break;
				case 0b00000001:
					if(bldcsensored_direction == BLDCSENSORED_DIRECTIONCW) { //direction CW
						BLDCSENSORED_001CW;
					} else if (bldcsensored_direction == BLDCSENSORED_DIRECTIONCCW) { //direction CCW
						BLDCSENSORED_001CCW;
					}
					break;
			}
		}
	}

}

#endif
