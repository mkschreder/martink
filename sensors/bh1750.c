/*
	Light sensor driver

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

#include "bh1750.h"

//resolution modes
#define BH1750_MODEH 0x10 //continuously h-resolution mode, 1lx resolution, 120ms
#define BH1750_MODEH2 0x11 //continuously h-resolution mode, 0.5lx resolution, 120ms
#define BH1750_MODEL 0x13 //continuously l-resolution mode, 4x resolution, 16ms
//define active resolution mode
#define BH1750_MODE BH1750_MODEH

void bh1750_init(struct bh1750 *self, i2c_dev_t i2c, uint8_t addr) {
	self->i2c = i2c;
	self->addr = addr;
	uint8_t mode = BH1750_MODE; 
	i2c_start_write(i2c, addr, &mode, 1);
	i2c_stop(i2c); 
}

uint16_t bh1750_read_intensity_lux(struct bh1750 *self) {
	uint16_t ret = 0;
	
	i2c_start_read(self->i2c, self->addr, (uint8_t*)&ret, 2);
	i2c_stop(self->i2c);
	return ret / 1.2f; 
}
