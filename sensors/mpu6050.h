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


struct mpu6050{
	i2c_dev_t i2c;
	uint8_t addr; 
}; 

//definitions
#define MPU6050_ADDR (0x68 <<1 ) //device address - 0x68 pin low (GND), 0x69 pin high (VCC)
#define MPU6050_GETATTITUDE 0

//functions
void mpu6050_init(struct mpu6050 *self, i2c_dev_t i2c, uint8_t addr);
uint8_t mpu6050_probe(struct mpu6050 *self);

//#if MPU6050_GETATTITUDE == 0
void mpu6050_readRawAcc(struct mpu6050 *self, int16_t* ax, int16_t* ay, int16_t* az);
void mpu6050_readRawGyr(struct mpu6050 *self, int16_t* gx, int16_t* gy, int16_t* gz);
void mpu6050_convertAcc(struct mpu6050 *self, int16_t ax, int16_t ay, int16_t az, float *axg, float *ayg, float *azg);
void mpu6050_convertGyr(struct mpu6050 *self, int16_t gx, int16_t gy, int16_t gz, float *gxd, float *gyd, float *gyz);
//void mpu6050_getRawData(struct mpu6050 *self, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
/*void mpu6050_convertData(struct mpu6050 *self, 
	int16_t ax, int16_t ay, int16_t az, 
	int16_t gx, int16_t gy, int16_t gz, 
	float *axg, float *ayg, float *azg, 
	float *gxd, float *gyd, float *gyz
)*/
//void mpu6050_getConvAcc(struct mpu6050 *self, double* axg, double* ayg, double* azg);
//void mpu6050_getConvGyr(struct mpu6050 *self, double* gxds, double* gyds, double* gzds);
//#endif

/*
void mpu6050_setSleepDisabled(struct mpu6050 *self);
void mpu6050_setSleepEnabled(struct mpu6050 *self);

int8_t mpu6050_readBytes(struct mpu6050 *self, uint8_t regAddr, uint8_t length, uint8_t *data);
int8_t mpu6050_readByte(struct mpu6050 *self, uint8_t regAddr, uint8_t *data);
void mpu6050_writeBytes(struct mpu6050 *self, uint8_t regAddr, uint8_t length, uint8_t* data);
void mpu6050_writeByte(struct mpu6050 *self, uint8_t regAddr, uint8_t data);
int8_t mpu6050_readBits(struct mpu6050 *self, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
int8_t mpu6050_readBit(struct mpu6050 *self, uint8_t regAddr, uint8_t bitNum, uint8_t *data);
void mpu6050_writeBits(struct mpu6050 *self, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
void mpu6050_writeBit(struct mpu6050 *self, uint8_t regAddr, uint8_t bitNum, uint8_t data);

#if MPU6050_GETATTITUDE == 1
void mpu6050_updateQuaternion(struct mpu6050 *self);
void mpu6050_getQuaternion(struct mpu6050 *self, double *qw, double *qx, double *qy, double *qz);
void mpu6050_getRollPitchYaw(struct mpu6050 *self, double *pitch, double *roll, double *yaw);
#endif

#if MPU6050_GETATTITUDE == 2
void mpu6050_writeWords(struct mpu6050 *self, uint8_t regAddr, uint8_t length, uint16_t* data);
void mpu6050_setMemoryBank(struct mpu6050 *self, uint8_t bank, uint8_t prefetchEnabled, uint8_t userBank);
void mpu6050_setMemoryStartAddress(struct mpu6050 *self, uint8_t address);
void mpu6050_readMemoryBlock(struct mpu6050 *self, uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address);
uint8_t mpu6050_writeMemoryBlock(struct mpu6050 *self, const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, uint8_t verify, uint8_t useProgMem);
uint8_t mpu6050_writeDMPConfigurationSet(struct mpu6050 *self, const uint8_t *data, uint16_t dataSize, uint8_t useProgMem);
uint16_t mpu6050_getFIFOCount(struct mpu6050 *self);
void mpu6050_getFIFOBytes(struct mpu6050 *self, uint8_t *data, uint8_t length);
uint8_t mpu6050_getIntStatus(struct mpu6050 *self);
void mpu6050_resetFIFO(struct mpu6050 *self);
//base dmp
uint8_t mpu6050_dmpInitialize(struct mpu6050 *self);
void mpu6050_dmpEnable(struct mpu6050 *self);
void mpu6050_dmpDisable(struct mpu6050 *self);
void mpu6050_getQuaternion(struct mpu6050 *self, const uint8_t* packet, double *qw, double *qx, double *qy, double *qz);
void mpu6050_getRollPitchYaw(struct mpu6050 *self, double qw, double qx, double qy, double qz, double *roll, double *pitch, double *yaw);
uint8_t mpu6050_getQuaternionWait(struct mpu6050 *self, double *qw, double *qx, double *qy, double *qz);
#endif
*/
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
