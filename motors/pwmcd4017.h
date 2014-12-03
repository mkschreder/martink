/*
pwmcd4017 lib 0x01

copyright (c) Davide Gironi, 2012

References:
  Marcelo Politzer Couto library,
  https://github.com/mpolitzer/caranguelo_de_komodo/

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <avr/io.h>

#ifndef PWMCD4017_H_
#define PWMCD4017_H_

#define PWMCD4017_RSTDDR DDRD
#define PWMCD4017_RSTPORT PORTD
#define PWMCD4017_RSTPIN PD7 //the reset pin

extern void pwmcd4017_init(void);
extern void pwmcd4017_set(uint8_t id, uint16_t deg);

//enable or disable high accurancy
#define PWMCD4017_HIGHACCU 1

//setup values here
#if PWMCD4017_HIGHACCU == 1
#define PWMCD4017_CHIP 1 //number of chip (must be 1)
#define PWMCD4017_CHIPPIN 10 //number of pin per chip, for cd4017 is 10
//we have to define the chip pin here
//chip 1 settings
#define PWMCD4017_CLK0DDR DDRD
#define PWMCD4017_CLK0PORT PORTD
#define PWMCD4017_CLK0PIN PD6
#else
#define PWMCD4017_CHIP 4 //number of chip (max 4)
#define PWMCD4017_CHIPPIN 10 //number of pin per chip, for cd4017 is 10
//we have to define the chip pin here
//chip 1 settings
#define PWMCD4017_CLK0DDR DDRD
#define PWMCD4017_CLK0PORT PORTD
#define PWMCD4017_CLK0PIN PD6
//chip 2 settings
#define PWMCD4017_CLK1DDR DDRD
#define PWMCD4017_CLK1PORT PORTD
#define PWMCD4017_CLK1PIN PD5
//chip 3 settings
#define PWMCD4017_CLK2DDR DDRD
#define PWMCD4017_CLK2PORT PORTD
#define PWMCD4017_CLK2PIN PD4
//chip 4 settings
#define PWMCD4017_CLK3DDR DDRD
#define PWMCD4017_CLK3PORT PORTD
#define PWMCD4017_CLK3PIN PD3
#endif

//all the math here is 16Mhz based
#if PWMCD4017_HIGHACCU == 1
//freq = FCPU/1+(prescaler)
//PWMCD4017_FREQUSREAL is the real precision = 1/freq
//1/16000000Hz = 0.062us
#define PWMCD4017_FREQUSREAL 0.062
//PWMCD4017_FREQUS is the selected precision = 1/freq
//lets suppose we want interrupt routine every 1us
#define PWMCD4017_FREQUS 1.0
//PWMCD4017_CLOCKDIFF is the multiplicator we use to set the clock from the PWMCD4017_FREQUS to
//PWMCD4017_FREQUSREAL value
//it is defined as (int)(PWMCD4017_FREQUS / PWMCD4017_FREQUSREAL)
//es. (int)(1us / 0.062us) = 16
#define PWMCD4017_CLOCKDIFF 16
//PWMCD4017_CLOCKMAX define the max value for the clock
//for a 10bit timer preloader can not exceed 65535
//it should be <= than 65535/PWMCD4017_CLOCKDIFF
//es. 65535/16 = 4095
#define PWMCD4017_CLOCKMAX 4095
#else
//PWMCD4017_FREQUS is the selected precision = 1/freq
//freq = FCPU/1+(prescaler*ICR1)
//note: ICR1 can not be small, because we have some commands to execute during the interrupt routine
//80000Hz = 16000000Hz/1+(1*198)
#define TIMER1_ICR1 198
//1/80000Hz = 0.0000125 = 12.5us
#define PWMCD4017_FREQUS 12.5
#endif

#if PWMCD4017_HIGHACCU == 1
	#if PWMCD4017_CHIP > 1
	#error "max number of chip"
	#endif
#else
	#if PWMCD4017_CHIP > 4
	#error "max number of chip"
	#endif
#endif

#define TIMER1_PRESCALER (1<<CS10) //prescaler

//struct definition
typedef struct {
	uint16_t clock[PWMCD4017_CHIPPIN];
	uint16_t clockcounter;
	uint8_t currentpin;
	volatile uint8_t *clkddr;
	volatile uint8_t *clkport;
	uint8_t clkpin;
} pwmcd4017_arrayt;

volatile pwmcd4017_arrayt pwmcd4017_array[PWMCD4017_CHIP];

#endif
