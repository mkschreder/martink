/*
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
	* this library is a porting of the bmp085driver 0.4 ardunio library
    http://code.google.com/p/bmp085driver/
	* Davide Gironi, original implementation
*/

#ifndef BMP085_H_
#define BMP085_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMP085_ADDR (0x77<<1) //0x77 default I2C address

struct bmp085 {
	i2c_dev_t i2c;
	uint8_t addr;
	int regac1, regac2, regac3, regb1, regb2, regmb, regmc, regmd;
	unsigned int regac4, regac5, regac6;
};

//functions
/// inits the device over the interface supplied 
void bmp085_init(struct bmp085 *self, i2c_dev_t i2c, uint8_t addr);
/// returns pressure 
long bmp085_read_pressure(struct bmp085 *self);
/// returns altitude
float bmp085_read_altitude(struct bmp085 *self);
/// returns temperature
float bmp085_read_temperature(struct bmp085 *self);

#ifdef __cplusplus
}
#endif
#endif
