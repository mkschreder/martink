/**
	output shift register driver for generic parallel port

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

//setup ports
#define L74HC595_DDR DDRB
#define L74HC595_PORT PORTB
#define L74HC595_STCPIN PB1
//#define L74HC595_CEPIN PB2
struct l74hc595 {
	serial_dev_t spi;
	pio_dev_t gpio;
	gpio_pin_t ce_pin;
	gpio_pin_t stc_pin;
}; 

void l74hc595_init(struct l74hc595 *self, serial_dev_t spi,
	pio_dev_t gpio, gpio_pin_t ce_pin, gpio_pin_t stc_pin);
void l74hc595_write(struct l74hc595 *self, uint8_t val);
 
