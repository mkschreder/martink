/*
fs300a water flow sensor lib 0x01

copyright (c) Davide Gironi, 2012

References:
  * http://www.seeedstudio.com/wiki/G3/4_Water_Flow_sensor

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef FS300A_H_
#define FS300A_H_

#include <stdio.h>
#include <avr/io.h>

//setup button port
#define FS300A_DDR DDRC
#define FS300A_PIN PINC
#define FS300A_PORT PORTC
#define FS300A_INPUT1 PC5

/*
 * timer setting
 * TIMER0_MSREPEATMS is the frequency of the timer
 * TIMER0_FS300ARESET is the countdown timer and it is = 1/TIMER0_MSREPEATMS
 * freq = 1 / time
 * es. 500hz = 1 / 0.002 (2ms)
 * timerfreq = (FCPU / prescaler) / timerscale
 *     timerscale 8-bit = 256
 * es. 488 = (1000000 / 8) / 256
 *     1 / 488 = 0.002s
 */
#define TIMER0_PRESCALER (1<<CS01) //prescaler 8
#define TIMER0_MSREPEATMS 2 //2ms
#define TIMER0_FS300ARESET 500 //1 / 0.002 = 500

//FS300A_CONVERSIONSENSITIVITY is the gain to compute the pulse frequency in horizontal test
#define FS300A_CONVERSIONSENSITIVITY 5.5

extern void fs300a_init(void);
extern uint16_t fs300a_getflow(void);

#endif
