/*
stepper04multi lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef STEPPER04MULTI_H
#define STEPPER04MULTI_H

//setup even the l74hc595 library

//number of motor to setup, max 12
//read timer notes before edit here
#define STEPPER04MULTI_MOTORNUM 12

//motor types and lookups
#define STEPPER04MULTI_MOTORTYPEUNIPOLARHALF 1
#define STEPPER04MULTI_MOTORTYPEUNIPOLARHALF_LOOKUP {0b00001000, 0b00001010, 0b00000010, 0b00000110, 0b00000100, 0b00000101, 0b00000001, 0b00001001}
#define STEPPER04MULTI_MOTORTYPEUNIPOLARFULL 2
#define STEPPER04MULTI_MOTORTYPEUNIPOLARFULL_LOOKUP {0b00001001, 0b00001010, 0b00000110, 0b00000101}
#define STEPPER04MULTI_MOTORTYPEBIPOLARHALF 3
#define STEPPER04MULTI_MOTORTYPEBIPOLARHALF_LOOKUP {0b00001000, 0b00001100, 0b00000100, 0b00000110, 0b00000010, 0b00000011, 0b00000001, 0b00001001}
#define STEPPER04MULTI_MOTORTYPEBIPOLARFULL 4
#define STEPPER04MULTI_MOTORTYPEBIPOLARFULL_LOOKUP {0b00001010, 0b00000110, 0b00000101, 0b00001001}

//setup motor type
#define STEPPER04MULTI_MOTOR01_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR02_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR03_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR04_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR05_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR06_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR07_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR08_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR09_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR10_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR11_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR12_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR13_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR14_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR15_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF
#define STEPPER04MULTI_MOTOR16_TYPE STEPPER04MULTI_MOTORTYPEBIPOLARHALF

//motor definition
typedef struct {
	uint8_t direction; //movement direction
	uint8_t *lookup; //lookup
	uint8_t lookupstep; //current lookup step
	uint8_t lookupstepnum; //total lookup steps
	uint16_t steptot; //number of step to run
	uint16_t stepdelay; //current delay for the lookup step
	uint16_t stepdelaytot; //total delay for the lookup step
	uint8_t l74hc595pin1; //pin 1 on the l74hc595 chain
	uint8_t l74hc595pin2; //pin 2 on the l74hc595 chain
	uint8_t l74hc595pin3; //pin 3 on the l74hc595 chain
	uint8_t l74hc595pin4; //pin 4 on the l74hc595 chain
} stepper04multim;

//direction
#define STEPPER04MULTI_DIRECTIONFORWARD 1
#define STEPPER04MULTI_DIRECTIONBACKWARD 2

//timer notes:
//experimental test runs 12 motors with 256us step precision @ 8mhz
//TIMER0_USDELAY is lower value for a step to come
//during a timer interrupt a function computes the actual pin status for every motor
//we have to be sure (i.e. test) that the timer has enough time to complete this computation
//we can not set a low prescaler(TIMER0_PRESCALER), and a low TIMER0_USDELAY if we have many motor
/*
 * timerfreq = (FCPU / prescaler) / timerscale
 *     timerscale 8-bit = 256
 * es. 3906.25 = (8000000 / 8) / 256
 *     1 / 3906.25 = 0.000256s = 256us
 */
#define TIMER0_PRESCALER (1<<CS01) //prescaler
#define TIMER0_USDELAY 256

//functions
extern void stepper04multi_init(void);
extern void stepper04multi_setsteptime(uint8_t motorid, uint16_t usdelay);
extern void stepper04multi_gostop(uint8_t motorid);
extern void stepper04multi_goforward(uint8_t motorid, uint16_t steps);
extern void stepper04multi_gobackward(uint8_t motorid, uint16_t steps);
extern uint16_t stepper04multi_getstep(uint8_t motorid);

#endif
