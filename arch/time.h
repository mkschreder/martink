/**
	This file is part of martink project.

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

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "types.h"

#define timestamp_init() (tsc_init())
#define timestamp_now() (tsc_read())

#define timestamp_ticks_to_us(ticks) (tsc_ticks_to_us((timestamp_t)(ticks)))
#define timestamp_us_to_ticks(us) (tsc_us_to_ticks((timestamp_t)(us)))

#define timestamp_before(unknown, known) ((timestamp_t)(unknown) - (timestamp_t)(known) < 0)
#define timestamp_after(a,b) timestamp_before(b, a)
#define timestamp_from_now_us(us) ((timestamp_t)(tsc_read() + tsc_us_to_ticks((timestamp_t)us)))
// timeout expired: can handle overflow of timer correctly
#define timestamp_expired(timeout) (timestamp_after(tsc_read(), timeout))

void timestamp_delay_us(timestamp_t usec);

#define delay_us(usec) timestamp_delay_us(usec)
#define delay_ms(msec) delay_us((msec) * 1000UL)

#define timestamp_ticks_since(timestamp) (tsc_read() - (timestamp_t)(timestamp))

void time_init(void); 

#ifdef __cplusplus
}
#endif
