/**
	8 bit multiplexer driver

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

#ifndef L74HC4051_H_
#define L74HC4051_H_

#define L74HC4051_MAXCH 8

struct l74hc4051 {
	pio_dev_t port; 
	gpio_pin_t s0_pin;
	gpio_pin_t s1_pin;
	gpio_pin_t s2_pin; 
};

void l74hc4051_init(struct l74hc4051 *self, pio_dev_t port,
		gpio_pin_t s0_pin, gpio_pin_t s1_pin, gpio_pin_t s2_pin);
void l74hc4051_set_channel(struct l74hc4051 *self, uint8_t channel);

#endif
