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

	Github: https://github.com/mkschreder

	Contributors:
	* John Ihlein - developing original driver
	* Martin K. Schröder - maintenance since Jan 2015
*/

#pragma once 

struct mpu6000{
	serial_dev_t port;
	pio_dev_t 	gpio; 
	gpio_pin_t 	cs_pin; 
}; 

void mpu6000_init(struct mpu6000 *self, serial_dev_t port, pio_dev_t gpio, gpio_pin_t cs_pin);
uint8_t mpu6000_probe(struct mpu6000 *self);

void mpu6000_readRawAcc(struct mpu6000 *self, int16_t* ax, int16_t* ay, int16_t* az);
void mpu6000_readRawGyr(struct mpu6000 *self, int16_t* gx, int16_t* gy, int16_t* gz);
void mpu6000_convertAcc(struct mpu6000 *self, int16_t ax, int16_t ay, int16_t az, float *axg, float *ayg, float *azg);
void mpu6000_convertGyr(struct mpu6000 *self, int16_t gx, int16_t gy, int16_t gz, float *gxd, float *gyd, float *gyz);
/*
void mpu6000_getRawData(struct mpu6000 *self, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
void mpu6000_convertData(struct mpu6000 *self, 
	int16_t ax, int16_t ay, int16_t az, 
	int16_t gx, int16_t gy, int16_t gz, 
	float *axg, float *ayg, float *azg, 
	float *gxd, float *gyd, float *gyz
); */
//void mpu6000_getConvAcc(struct mpu6000 *self, double* axg, double* ayg, double* azg);
//void mpu6000_getConvGyr(struct mpu6000 *self, double* gxds, double* gyds, double* gzds);
