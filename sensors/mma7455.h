/*
	MMA7455 accelerometer driver
	
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

#ifndef MMA7455_H_
#define MMA7455_H_

//definitions
#define MMA7455_ADDR (0x1D<<1) //device address

struct mma7455 {
	i2c_dev_t i2c;
	uint8_t addr;
	uint8_t mode; 
};

//functions declarations
void mma7455_init(struct mma7455 *self, i2c_dev_t i2c, uint8_t addr, uint8_t mode);
void mma7455_getdata(struct mma7455 *self, float *ax, float *ay, float *az);
#if MMA7455_GETATTITUDE == 1
void mma7455_getpitchroll(struct mma7455 *self, float ax, float ay, float az, float *pitch, float *roll);
#endif

#endif
