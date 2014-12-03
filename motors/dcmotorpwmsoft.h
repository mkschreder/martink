/*
dcmotorpwmsoft lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef DCMOTORPWMSOFT_H
#define DCMOTORPWMSOFT_H

//number of motor
#define DCMOTORPWMSOFT_MOTORNUM 2

#define DCMOTORPWMSOFT_MOTORNUMMAX 4 //max number of motor
#if DCMOTORPWMSOFT_MOTORNUM > DCMOTORPWMSOFT_MOTORNUMMAX
	#error "max number of motor"
#endif

/*
 * timer setting
 * timerfreq = (FCPU / prescaler) / timerscale
 *     timerscale 8-bit = 256
 * es. 3906 = (1000000 / 1) / 256
 *     1 / 3906 = 0.0002502s = 0.250ms = 250us
 */
#define TIMER0_PRESCALER (1<<CS00) //prescaler 1
#define TIMER0_USDELAY 250 //delay us

//define the max value for step, and the frequency for the motor
//DCMOTORPWMSOFT_MAXVAL = (1/freq)/(TIMER0_USDELAY/(1000*1000))
//es. select 10hz as frequency, 400 = (1/10hz) / (250/(1000*1000))
//DCMOTORPWMSOFT_MAXVAL should be > 100 (which is the velocity max value)
#define DCMOTORPWMSOFT_MAXVAL 400
#define DCMOTORPWMSOFT_MINVAL 300
//motor array
typedef struct {
	volatile uint8_t *ddr;
	volatile uint8_t *port;
	uint8_t pinA;
	uint8_t pinB;
	uint16_t step;
	uint16_t vel;
	volatile uint8_t direction;
} dcmotorpwmsoft_arrayt;

volatile dcmotorpwmsoft_arrayt dcmotorpwmsoft_array[DCMOTORPWMSOFT_MOTORNUM];

//definitions
#define DCMOTORPWMSOFT_DIRECTIONFORWARD 1
#define DCMOTORPWMSOFT_DIRECTIONBACKWARD 2
#define DCMOTORPWMSOFT_DIRECTIONSTOP 0

//functions
extern void dcmotorpwmsoft_init(void);
extern void dcmotorpwmsoft_gostop(uint8_t motorid);
extern void dcmotorpwmsoft_goforward(uint8_t motorid, uint16_t velocity);
extern void dcmotorpwmsoft_gobackward(uint8_t motorid, uint16_t velocity);

#endif
