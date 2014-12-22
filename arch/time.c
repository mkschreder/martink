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
	timestamp_t t = timestamp_from_now_us(usec); 
	while(!timestamp_expired(t));
}

static void __init time_init(void){
#ifdef CONFIG_TIMESTAMP_COUNTER
	timestamp_init();
	kdebug("TIME: using timestamp counter!\n");
#endif
}
