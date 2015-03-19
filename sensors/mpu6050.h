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

#include <kernel/thread.h>
#include <kernel/dev/i2c.h>

enum {
	MPU6050_REG_PWR_MGMT_1, 
	MPU6050_REG_CONFIG,
	MPU6050_REG_SMPLRT_DIV,
	MPU6050_REG_GYRO_CONFIG,
	MPU6050_REG_ACCEL_CONFIG,
	MPU6050_REG_INT_PIN_CFG,
	MPU6050_REG_USER_CTRL,
	MPU6050_REG_COUNT
}; 

struct mpu6050{
	io_dev_t dev;
	
	// cached data
	int16_t raw[6]; 
	
	// threads
	struct libk_thread kthread; 
	struct async_task task; 
	
	//struct pt uthread, rthread, wthread, ithread; 
	uint8_t buf[6]; // i2c buffer
	
	timestamp_t time; // for time keeping
	unsigned int count; 
	uint8_t state; // status 
}; 

//definitions
#define MPU6050_ADDR (0x68 <<1 ) //device address - 0x68 pin low (GND), 0x69 pin high (VCC)
#define MPU6050_GETATTITUDE 0

//functions
void mpu6050_init(struct mpu6050 *self, io_dev_t device);
void mpu6050_deinit(struct mpu6050 *self); 
//void mpu6050_update(struct mpu6050 *self); 
uint8_t mpu6050_probe(struct mpu6050 *self);

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
