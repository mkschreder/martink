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

/// this holds the number of overflows of timer 1 (which counts clock ticks)
volatile timestamp_t _tsc_ovf = 0;

ISR (TIMER1_OVF_vect)
{
	_tsc_ovf++;
}
