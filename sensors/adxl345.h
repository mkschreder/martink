/**
	Accelerometer driver

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

#define ADXL345_ADDR (0x53<<1) //device address

struct adxl345 {
	i2c_dev_t i2c;
	uint8_t addr; 
};

void adxl345_init(struct adxl345 *self, i2c_dev_t i2c, uint8_t addr);
int8_t adxl345_read_raw(struct adxl345 *self, int16_t *ax, int16_t *ay, int16_t *az); 
int8_t adxl345_read_adjusted(struct adxl345 *self, float *ax, float *ay, float *az); 
