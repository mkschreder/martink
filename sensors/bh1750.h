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

#ifndef BH1750_H_
#define BH1750_H_

#define BH1750_ADDR (0x23<<1) //device address

struct bh1750 {
	i2c_dev_t i2c;
	uint8_t addr; 
};

void bh1750_init(struct bh1750 *self, i2c_dev_t i2c, uint8_t addr);
uint16_t bh1750_read_intensity_lux(struct bh1750 *self);

#endif
