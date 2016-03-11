/**
	This file is part of martink project.
	
	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

	This software is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#define DEBUG(...) do {} while(0)
#define NATIVE_USLEEP(x) do { int __attribute((unused)) y = x; } while(0)

#define ATOMIC_BLOCK(...) 

static inline int32_t tsc_read(void){
	int32_t r;
	asm volatile ("rsr %0, ccount" : "=r"(r));
	return r;
}

#define tsc_ticks_to_us(ticks) (ticks)
#define tsc_us_to_ticks(ticks) (ticks)

#define sei()
#define cli()

#define printf(...) do {} while(0)
