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


enum {
	GP_INPUT,
	GP_OUTPUT,
	GP_AUX_A, 
	GP_AUX_B, 
	GP_AUX_C, 
	GP_AUX_D
};

/*
#define gpio_set(pin, value) PFCALL(gpio, set, value)
#define gpio_set_direction(pin, dir)
#define gpio_set_pullup(pin, pull)
#define gpio_set_function(pin, func)
*/
