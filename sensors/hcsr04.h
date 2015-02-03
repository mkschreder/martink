/*
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

	Github: https://github.com/mkschreder

	Contributors:
	* Martin K. Schröder - original driver
*/

#ifndef HCSR04_H
#define HCSR04_H

struct hcsr04 {
	pio_dev_t gpio;
	gpio_pin_t trigger_pin, echo_pin;
	uint8_t state;
	timestamp_t pulse_timeout; 
	int16_t distance; 
};

void hcsr04_init(struct hcsr04 *self, pio_dev_t gpio,
	gpio_pin_t trigger_pin, gpio_pin_t echo_pin); 
int16_t hcsr04_read_distance_in_cm(struct hcsr04 *self); 

#endif
