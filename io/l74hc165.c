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


#include <stdio.h>
#include <string.h>
#include <arch/soc.h>

#include "l74hc165.h"

/*
 * init the shift register
 */
void l74hc165_init(struct l74hc165 *self, pio_dev_t port,
		gpio_pin_t clock_pin, gpio_pin_t load_pin, gpio_pin_t data_pin) {
	self->port = port;
	self->clock_pin = clock_pin;
	self->load_pin = load_pin;
	self->data_pin = data_pin;
	
	pio_configure_pin(port, clock_pin, GP_OUTPUT);
	pio_configure_pin(port, load_pin, GP_OUTPUT);
	pio_configure_pin(port, data_pin, GP_INPUT | GP_PULLUP);
	pio_write_pin(port, clock_pin, 0);
	pio_write_pin(port, load_pin, 0);
}

/*
 * shift in data
 */
void l74hc165_read(struct l74hc165 *self, uint8_t *bytearray, uint8_t count) {
	//parallel load to freeze the state of the data lines
	pio_write_pin(self->port, self->load_pin, 0); 
	delay_us(5);
	pio_write_pin(self->port, self->load_pin, 1); 
	for(uint8_t i = 0; i < count; i++){
		//iterate through the bits in each registers
		uint8_t currentbyte = 0;
		for(uint8_t j = 0; j < 8; j++){
			uint8_t data = (pio_read_pin(self->port, self->data_pin))?1:0; 
			currentbyte |= (data << (7-j));
					//get next
			pio_write_pin(self->port, self->clock_pin, 0); 
			delay_us(5);
			pio_write_pin(self->port, self->clock_pin, 1); 
		}
		memcpy(&bytearray[i], &currentbyte, 1);
	}
}
