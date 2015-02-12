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

#include <inttypes.h>

#include <arch/soc.h>

void timestamp_delay_us(timestamp_t usec) {
	volatile timestamp_t t = timestamp_from_now_us(usec); 
	//printf("waiting.. %d %d\n", (int)t, (int)timestamp_now()); 
	while(!timestamp_expired(t)); // printf("waiting.. %d %d\n", (int)t, (int)timestamp_now()); 
}

timestamp_t timestamp_ticks_to_us(timestamp_t ticks) {
	return tsc_ticks_to_us((timestamp_t)(ticks)); 
}

timestamp_t timestamp_us_to_ticks(timestamp_t us) {
	return tsc_us_to_ticks((timestamp_t)(us)); 
}

timestamp_t timestamp_from_now_us(timestamp_t us) {
	return (timestamp_t)(tsc_read() + timestamp_us_to_ticks((timestamp_t)us)); 
}

uint8_t timestamp_expired(timestamp_t timeout) {
	return timestamp_after(tsc_read(), timeout); 
}

timestamp_t timestamp_ticks_since(timestamp_t timestamp) {
	return tsc_read() - (timestamp_t)(timestamp); 
}

void initproc time_init(void){
#ifdef CONFIG_TIMESTAMP_COUNTER
	timestamp_init();
#endif
	sei(); 
}
