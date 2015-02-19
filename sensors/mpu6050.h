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
	* Davide Gironi - developing original driver
	* Martin K. Schröder - maintenance since Oct 2014

	References:
		- most of the code is a port of the arduino mpu6050 library by Jeff Rowberg
			https://github.com/jrowberg/i2cdevlib
		- Mahony complementary filter for attitude estimation
			http://www.x-io.co.uk
*/


#ifndef MPU6050_H_
#define MPU6050_H_

#ifdef __cplusplus
"C" {
#endif

#include <thread/pt.h>

struct mpu6050{
	i2c_dev_t i2c;
	uint8_t addr;
	int16_t raw_ax, raw_ay, raw_az; 
	int16_t raw_gx, raw_gy, raw_gz; 
	struct pt thread, rbthread; 
	uint8_t buffer[6]; 
	timestamp_t time; 
}; 

//definitions
#define MPU6050_ADDR (0x68 <<1 ) //device address - 0x68 pin low (GND), 0x69 pin high (VCC)
#define MPU6050_GETATTITUDE 0

//functions
void mpu6050_init(struct mpu6050 *self, i2c_dev_t i2c, uint8_t addr);
uint8_t mpu6050_probe(struct mpu6050 *self);
PT_THREAD(mpu6050_thread(struct mpu6050 *self)); 

//#if MPU6050_GETATTITUDE == 0
void mpu6050_readRawAcc(struct mpu6050 *self, int16_t* ax, int16_t* ay, int16_t* az);
void mpu6050_readRawGyr(struct mpu6050 *self, int16_t* gx, int16_t* gy, int16_t* gz);
void mpu6050_convertAcc(struct mpu6050 *self, int16_t ax, int16_t ay, int16_t az, float *axg, float *ayg, float *azg);
void mpu6050_convertGyr(struct mpu6050 *self, int16_t gx, int16_t gy, int16_t gz, float *gxd, float *gyd, float *gyz);

/// temperature compensation offsets
int8_t mpu6050_getTCXGyroOffset(struct mpu6050 *self);
void mpu6050_setTCXGyroOffset(struct mpu6050 *self, int8_t offset);
int8_t mpu6050_getTCYGyroOffset(struct mpu6050 *self);
void mpu6050_setTCYGyroOffset(struct mpu6050 *self, int8_t offset);
int8_t mpu6050_getTCZGyroOffset(struct mpu6050 *self);
void mpu6050_setTCZGyroOffset(struct mpu6050 *self, int8_t offset);

/// gyro offsets
void 		mpu6050_setXGyroOffset(struct mpu6050 *self, int16_t offset);
int16_t mpu6050_getXGyroOffset(struct mpu6050 *self); 
void 		mpu6050_setYGyroOffset(struct mpu6050 *self, int16_t offset);
int16_t mpu6050_getYGyroOffset(struct mpu6050 *self); 
void 		mpu6050_setZGyroOffset(struct mpu6050 *self, int16_t offset);
int16_t mpu6050_getZGyroOffset(struct mpu6050 *self); 

void 		mpu6050_setXAccOffset(struct mpu6050 *self, int16_t offset);
int16_t mpu6050_getXAccOffset(struct mpu6050 *self); 
void 		mpu6050_setYAccOffset(struct mpu6050 *self, int16_t offset);
int16_t mpu6050_getYAccOffset(struct mpu6050 *self); 
void 		mpu6050_setZAccOffset(struct mpu6050 *self, int16_t offset);
int16_t mpu6050_getZAccOffset(struct mpu6050 *self); 

#ifdef __cplusplus
}
#endif

#endif
