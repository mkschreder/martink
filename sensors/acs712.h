/*
	Current sensor driver

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

	Special thanks to:
	* Davide Gironi, original implementation
*/
#pragma once

#include <arch/soc.h>

#include <kernel/thread.h>

//defined sensitivity
#define ACS712_SENSITIVITY5 0.185
#define ACS712_SENSITIVITY20 0.100
#define ACS712_SENSITIVITY30 0.066

//setup sensitivity
#define ACS712_SENSITIVITY ACS712_SENSITIVITY30

struct acs712 {
	uint8_t adc_chan; 
	uint16_t interval; 
	timestamp_t time; 
	struct libk_thread thread; 
	uint16_t raw_value; 
}; 

void acs712_init(struct acs712 *self, uint8_t adc_chan, uint16_t read_interval_us);
void acs712_update(struct acs712 *self);
float acs712_read_current(struct acs712 *self, float sensitivity, float vcc_volt);
