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

#include <arch/soc.h>

void _gpio_write_pin(struct parallel_interface *self, uint16_t pin, uint8_t value){
	gpio_write_pin(pin, value);
}

uint8_t _gpio_read_pin(struct parallel_interface *self, uint16_t pin){
	return gpio_read_pin(pin);
}

uint8_t _gpio_configure_pin(struct parallel_interface *self, uint16_t pin, uint16_t flags){
	gpio_configure(pin, flags);
	return 0; 
}

uint8_t _gpio_get_pin_status(struct parallel_interface *self, uint16_t pin, timestamp_t *t_up, timestamp_t *t_down){
	return gpio_get_status(pin, t_up, t_down);
}

uint8_t _gpio_write_word(struct parallel_interface *self, uint16_t addr, uint32_t value){
	return gpio_write_word(addr, value);
}

uint8_t _gpio_read_word(struct parallel_interface *self, uint16_t addr, uint32_t *output){
	return gpio_read_word(addr, output);
}

struct parallel_interface gpio_get_parallel_interface(void){
	return (struct parallel_interface){
		.configure_pin = _gpio_configure_pin, 
		.write_pin = 		_gpio_write_pin,
		.read_pin = 		_gpio_read_pin,
		.write_word = 	_gpio_write_word,
		.read_word = 		_gpio_read_word,
		.get_pin_status = _gpio_get_pin_status
	}; 
}

static void __init _gpio_init(void){
	kdebug("GPIO: chip has %d gpio pins\n", GPIO_COUNT); 
}
