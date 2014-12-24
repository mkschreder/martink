/**
	input shift register driver for generic parallel port

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

#ifndef L74HC165_H_
#define L74HC165_H_

struct l74hc165 {
	pio_dev_t port; 
	gpio_pin_t clock_pin;
	gpio_pin_t load_pin;
	gpio_pin_t data_pin; 
};

void l74hc165_init(struct l74hc165 *self, pio_dev_t port,
		gpio_pin_t clock_pin, gpio_pin_t load_pin, gpio_pin_t data_pin);
void l74hc165_read(struct l74hc165 *self, uint8_t *data, uint8_t count);

#endif
