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

#include <stdio.h>

#include <arch/soc.h>

#include "l74hc595.h"

#ifndef CONFIG_L74HC595_STC_PIN
#define CONFIG_L74HC595_STC_PIN GPIO_NONE
#endif

#define spi_writereadbyte(b) (serial_putc(self->spi, b), serial_getc(self->spi))

#define L74HC595_CELo pio_write_pin(self->gpio, self->ce_pin, 0)
#define L74HC595_CEHi pio_write_pin(self->gpio, self->ce_pin, 1)

#define L74HC595_STCLo pio_write_pin(self->gpio, self->stc_pin, 0)
#define L74HC595_STCHi pio_write_pin(self->gpio, self->stc_pin, 1)
/*
 * init the shift register
 */
void l74hc595_init(struct l74hc595 *self, serial_dev_t spi, pio_dev_t gpio, gpio_pin_t ce_pin, gpio_pin_t stc_pin) {
	self->spi = spi;
	self->gpio = gpio; 
	self->stc_pin = stc_pin;
	self->ce_pin = ce_pin; 
	
	pio_configure_pin(gpio, ce_pin, GP_OUTPUT); 
	pio_configure_pin(gpio, stc_pin, GP_OUTPUT); 
	
	L74HC595_STCLo;  
}

void l74hc595_write(struct l74hc595 *self, uint8_t data) {
	L74HC595_CEHi; 
		spi_writereadbyte(data); 
		L74HC595_STCHi; 
		delay_us(1); // not needed but still for safety (16ns is minimum high period)
		L74HC595_STCLo;
	L74HC595_CELo; 
}
