/*
stepper02 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef STEPPER02_H
#define STEPPER02_H

//number of motor to setup, max 3
//read timer notes before edit here
#define STEPPER02_MOTORNUM 3

//motor types and lookups
#define STEPPER02_MOTORTYPEUNIPOLARHALF 1
#define STEPPER02_MOTORTYPEUNIPOLARHALF_LOOKUP {0b00001000, 0b00001010, 0b00000010, 0b00000110, 0b00000100, 0b00000101, 0b00000001, 0b00001001}
#define STEPPER02_MOTORTYPEUNIPOLARFULL 2
#define STEPPER02_MOTORTYPEUNIPOLARFULL_LOOKUP {0b00001001, 0b00001010, 0b00000110, 0b00000101}
#define STEPPER02_MOTORTYPEBIPOLARHALF 3
#define STEPPER02_MOTORTYPEBIPOLARHALF_LOOKUP {0b00001000, 0b00001100, 0b00000100, 0b00000110, 0b00000010, 0b00000011, 0b00000001, 0b00001001}
#define STEPPER02_MOTORTYPEBIPOLARFULL 4
#define STEPPER02_MOTORTYPEBIPOLARFULL_LOOKUP {0b00001010, 0b00000110, 0b00000101, 0b00001001}

//setup motor type
#define STEPPER02_MOTOR1TYPE STEPPER02_MOTORTYPEBIPOLARHALF
#define STEPPER02_MOTOR2TYPE STEPPER02_MOTORTYPEBIPOLARHALF
#define STEPPER02_MOTOR3TYPE STEPPER02_MOTORTYPEBIPOLARHALF

//setup motor 1
#define STEPPER02_MOTOR1PIN1 PD2
#define STEPPER02_MOTOR1PORT1 PORTD
#define STEPPER02_MOTOR1DDR1 DDRD
#define STEPPER02_MOTOR1PIN2 PD3
#define STEPPER02_MOTOR1PORT2 PORTD
#define STEPPER02_MOTOR1DDR2 DDRD
#define STEPPER02_MOTOR1PIN3 PD4
#define STEPPER02_MOTOR1PORT3 PORTD
#define STEPPER02_MOTOR1DDR3 DDRD
#define STEPPER02_MOTOR1PIN4 PD5
#define STEPPER02_MOTOR1PORT4 PORTD
#define STEPPER02_MOTOR1DDR4 DDRD

//setup motor 2
#define STEPPER02_MOTOR2PIN1 PB0
#define STEPPER02_MOTOR2PORT1 PORTB
#define STEPPER02_MOTOR2DDR1 DDRB
#define STEPPER02_MOTOR2PIN2 PB1
#define STEPPER02_MOTOR2PORT2 PORTB
#define STEPPER02_MOTOR2DDR2 DDRB
#define STEPPER02_MOTOR2PIN3 PB2
#define STEPPER02_MOTOR2PORT3 PORTB
#define STEPPER02_MOTOR2DDR3 DDRB
#define STEPPER02_MOTOR2PIN4 PB3
#define STEPPER02_MOTOR2PORT4 PORTB
#define STEPPER02_MOTOR2DDR4 DDRB

//setup motor 3
#define STEPPER02_MOTOR3PIN1 PC0
#define STEPPER02_MOTOR3PORT1 PORTC
#define STEPPER02_MOTOR3DDR1 DDRC
#define STEPPER02_MOTOR3PIN2 PC1
#define STEPPER02_MOTOR3PORT2 PORTC
#define STEPPER02_MOTOR3DDR2 DDRC
#define STEPPER02_MOTOR3PIN3 PC2
#define STEPPER02_MOTOR3PORT3 PORTC
#define STEPPER02_MOTOR3DDR3 DDRC
#define STEPPER02_MOTOR3PIN4 PC3
#define STEPPER02_MOTOR3PORT4 PORTC
#define STEPPER02_MOTOR3DDR4 DDRC

//motor definition
typedef struct {
	uint8_t direction; //movement direction
	uint8_t *lookup; //lookup
	uint8_t lookupstep; //current lookup step
	uint8_t lookupstepnum;  //total lookup step
	uint16_t stepdelay; //current delay for the lookup step
	uint16_t stepdelaytot; //total delay for the lookup step
	uint16_t steptot; //number of step to run
	uint8_t pin1; //pin 1
	volatile uint8_t *port1;
	volatile uint8_t *ddr1;
	uint8_t pin2; //pin 2
	volatile uint8_t *port2;
	volatile uint8_t *ddr2;
	uint8_t pin3; //pin 3
	volatile uint8_t *port3;
	volatile uint8_t *ddr3;
	uint8_t pin4; //pin 4
	volatile uint8_t *port4;
	volatile uint8_t *ddr4;
} stepper02m;

//direction
#define STEPPER02_DIRECTIONFORWARD 1
#define STEPPER02_DIRECTIONBACKWARD 2

//timer notes:
//experimental test runs 3 motors with 256us step precision @ 8@mhz
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
#define TIMER0_PRESCALER (1<<CS01) //prescaler 8
#define TIMER0_USDELAY 256 //delay us

//functions
extern void stepper02_init(void);
extern void stepper02_setsteptime(uint8_t motorid, uint16_t usdelay);
extern void stepper02_gostop(uint8_t motorid);
extern void stepper02_goforward(uint8_t motorid, uint16_t steps);
extern void stepper02_gobackward(uint8_t motorid, uint16_t steps);
extern uint16_t stepper02_getstep(uint8_t motorid);

#endif
