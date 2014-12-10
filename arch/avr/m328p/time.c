/** 
 * 	Author: Martin K. Schröder 
 *  Date: 2014
 * 
 * 	info@fortmax.se
 */
 
#include <avr/io.h>
#include <avr/interrupt.h>

#include <arch/time.h>
#include "time.h"

// update this for correct amount of ticks in microsecon
#define TICKS_PER_US 2

/// this holds the number of overflows of timer 1 (which counts clock ticks)
static volatile uint32_t _timer1_ovf = 0;

void time_init(void){
	TCCR1A = 0; //_BV(WGM10) | _BV(WGM11); 
	//TCCR1B = _BV(CS10); //_BV(WGM13) | _BV(WGM12)  | _BV(CS10); // PWM OCRA, 8 ps
	TCCR1B = _BV(CS11); // clk/8 (2 ticks per us)
	TIMSK1 |= _BV(TOIE0);
	//OCR1A = TIMER_TICKS_PER_FRAME;
}

void time_reset(void){
	_timer1_ovf = 0;
	TCNT1 = 0; 
}

timeout_t time_us_to_clock(timeout_t us){
	return TICKS_PER_US * us; 
}

timeout_t time_clock_to_us(timeout_t clock){
	return clock / TICKS_PER_US; 
}

timeout_t time_get_clock(void){
	return TCNT1 + _timer1_ovf * 65535;
}

ISR (TIMER1_OVF_vect)
{
	_timer1_ovf++;
}
