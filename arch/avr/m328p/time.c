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
#endif
