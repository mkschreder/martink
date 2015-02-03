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

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder

	\file

	Architecture independent GPIO driver
*/
#pragma once

// GPIO pin functions
/// configures a gpio pin as input
#define GP_INPUT 		(0)
/// configures a gpio pin as output
#define GP_OUTPUT 	(1 << 0)
/// enables pull(up/down) on gpio pin
#define GP_PULL 		(1 << 1)
/// no pullup on gpio pin
#define GP_HIZ			(0)
/// enable pullup
#define GP_PULLUP 	(GP_PULL | (1 << 2))
/// enable pulldown (if arch supports it)
#define GP_PULLDOWN (GP_PULL | (0 << 2))
/// alternative function on the pin
#define GP_AF 	(1 << 3)
/// output open drain mode
#define GP_OPEN_DRAIN		(1 << 4)
/// analog input
#define GP_ANALOG (1 << 5)

/// pin change interrupt will run when pin gets toggled
/// make sure that pin change handler is defined
#define GP_PCINT		(1 << 3)

// GPIO status flags
/// gpio status: pin level has changed
#define GP_CHANGED 		(1 << 0)
/// gpio status: pin went high
#define GP_WENT_HIGH 	(GP_CHANGED | (1 << 1))
/// gpio status: pin went low
#define GP_WENT_LOW 	(GP_CHANGED | (1 << 2))

#define gpio_init() gpio_init_default()

#define gpio_set(pin) gpio_write_pin(pin, 1)
#define gpio_clear(pin) gpio_write_pin(pin, 0)

/// returns a standard parallel interface for the hardware gpio pins
/// as a \ref struct parallel_interface
pio_dev_t gpio_get_parallel_interface(void); 
/*
#define gpio_set(pin, value) PFCALL(gpio, set, value)
#define gpio_set_direction(pin, dir)
#define gpio_set_pullup(pin, pull)
#define gpio_set_function(pin, func)
*/
