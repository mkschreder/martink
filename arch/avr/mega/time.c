/**
	This file is part of martink kernel library

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

 
#include <avr/io.h>
#include <avr/interrupt.h>

#include <arch/soc.h>
#include "time.h"

#if defined(CONFIG_TIMESTAMP_COUNTER)
	/// this holds the number of overflows of timer 1 (which counts clock ticks)
	volatile timestamp_t _tsc_ovf = 0;

	ISR (TIMER1_OVF_vect)
	{
		_tsc_ovf++;
	}
	
	timestamp_t tsc_read(void){
		timestamp_t time; 
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			time = TCNT1 + _tsc_ovf * 65535L;
		}
		return time;
	}

	timestamp_t tsc_us_to_ticks(timestamp_t us) {
		return ((timestamp_t)(TSC_TICKS_PER_US * ((timestamp_t)us)));
	}

	timestamp_t tsc_ticks_to_us(timestamp_t ticks){
		return (((timestamp_t)ticks) / TSC_TICKS_PER_US);
	}

	static void __init tsc_init(void){
		timer1_mode(TIM1_MODE_NORMAL);
		timer1_set_clock(TSC_PRESCALER); 
		timer1_interrupt_overflow_on(); 
	}
#endif


