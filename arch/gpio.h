/**
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
*/

#pragma once

// GPIO pin functions
#define GP_INPUT 		(0)
#define GP_OUTPUT 	(1 << 0)
#define GP_PULL 		(1 << 1)
#define GP_HIZ			(0 << 1)
#define GP_PULLUP 	(GP_PULL | (1 << 2))
#define GP_PULLDOWN (GP_PULL | (0 << 2))
// pin change interrupt will run when pin gets toggled
#define GP_PCINT		(1 << 3)

// GPIO status flags
#define GP_CHANGED 		(1 << 0)
#define GP_WENT_HIGH 	(GP_CHANGED | (1 << 1))
#define GP_WENT_LOW 	(GP_CHANGED | (1 << 2))

/// returns a standard parallel interface for the hardware gpio pins
struct parallel_interface gpio_get_parallel_interface(void); 
/*
#define gpio_set(pin, value) PFCALL(gpio, set, value)
#define gpio_set_direction(pin, dir)
#define gpio_set_pullup(pin, pull)
#define gpio_set_function(pin, func)
*/
