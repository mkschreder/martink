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

#include <stdio.h>
#include <inttypes.h>

#include <arch/soc.h>
#include "l74hc4051.h"

/*
 * init the shift register
 */
void l74hc4051_init(struct l74hc4051 *self, pio_dev_t port,
		gpio_pin_t s0_pin, gpio_pin_t s1_pin, gpio_pin_t s2_pin) {
	self->port = port;
	self->s0_pin = s0_pin;
	self->s1_pin = s1_pin;
	self->s2_pin = s2_pin;
	
	pio_configure_pin(port, s0_pin, GP_OUTPUT);
	pio_configure_pin(port, s1_pin, GP_OUTPUT);
	pio_configure_pin(port, s2_pin, GP_OUTPUT);
	pio_write_pin(port, s0_pin, 0);
	pio_write_pin(port, s1_pin, 0);
	pio_write_pin(port, s2_pin, 0);
}

void l74hc4051_set_channel(struct l74hc4051 *self, uint8_t channel) {
	//bit 1
	if((channel & (1 << 0)) >> 0)
		pio_write_pin(self->port, self->s0_pin, 1);
	else
		pio_write_pin(self->port, self->s0_pin, 0);
	//bit 2
	if((channel & (1 << 1)) >> 1)
		pio_write_pin(self->port, self->s1_pin, 1);
	else
		pio_write_pin(self->port, self->s1_pin, 0);
	//bit 3
	if((channel & (1 << 2)) >> 2)
		pio_write_pin(self->port, self->s2_pin, 1);
	else
		pio_write_pin(self->port, self->s2_pin, 0);
}
