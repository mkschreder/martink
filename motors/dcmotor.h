/*
dcmotor lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef DCMOTOR_H
#define DCMOTOR_H

//stepper motor array dimension
#define DCMOTOR_MOTORNUM 2

//set dcmotor_setup() to define ports

//struct definition
typedef struct {
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	uint8_t pinA;
	uint8_t pinB;
} dcmotor_arrayt;

volatile dcmotor_arrayt dcmotor_array[DCMOTOR_MOTORNUM];

//functions
extern void dcmotor_init(void);
extern void dcmotor_gostop(uint8_t motorid);
extern void dcmotor_goforward(uint8_t motorid);
extern void dcmotor_gobackward(uint8_t motorid);

#endif
