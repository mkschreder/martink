/*
MPU6050 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - most of the code is a port of the arduino mpu6050 library by Jeff Rowberg
    https://github.com/jrowberg/i2cdevlib
  - Mahony complementary filter for attitude estimation
    http://www.x-io.co.uk
*/


#ifndef MPU6050_H_
#define MPU6050_H_

#ifdef __cplusplus
extern "C" {
#endif


struct mpu6050{
	struct packet_interface *port; 
}; 

//definitions
#define MPU6050_ADDR (0x68 <<1) //device address - 0x68 pin low (GND), 0x69 pin high (VCC)
#define MPU6050_GETATTITUDE 0

//functions
extern void mpu6050_init(struct mpu6050 *self, struct packet_interface *port);
extern uint8_t mpu6050_probe(struct mpu6050 *self);

#if MPU6050_GETATTITUDE == 0
extern void mpu6050_getRawData(struct mpu6050 *self, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
extern void mpu6050_getConvAcc(struct mpu6050 *self, double* axg, double* ayg, double* azg);
extern void mpu6050_getConvGyr(struct mpu6050 *self, double* gxds, double* gyds, double* gzds);
#endif

extern void mpu6050_setSleepDisabled(struct mpu6050 *self);
extern void mpu6050_setSleepEnabled(struct mpu6050 *self);

extern int8_t mpu6050_readBytes(struct mpu6050 *self, uint8_t regAddr, uint8_t length, uint8_t *data);
extern int8_t mpu6050_readByte(struct mpu6050 *self, uint8_t regAddr, uint8_t *data);
extern void mpu6050_writeBytes(struct mpu6050 *self, uint8_t regAddr, uint8_t length, uint8_t* data);
extern void mpu6050_writeByte(struct mpu6050 *self, uint8_t regAddr, uint8_t data);
extern int8_t mpu6050_readBits(struct mpu6050 *self, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
extern int8_t mpu6050_readBit(struct mpu6050 *self, uint8_t regAddr, uint8_t bitNum, uint8_t *data);
extern void mpu6050_writeBits(struct mpu6050 *self, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
extern void mpu6050_writeBit(struct mpu6050 *self, uint8_t regAddr, uint8_t bitNum, uint8_t data);

#if MPU6050_GETATTITUDE == 1
extern void mpu6050_updateQuaternion(struct mpu6050 *self);
extern void mpu6050_getQuaternion(struct mpu6050 *self, double *qw, double *qx, double *qy, double *qz);
extern void mpu6050_getRollPitchYaw(struct mpu6050 *self, double *pitch, double *roll, double *yaw);
#endif

#if MPU6050_GETATTITUDE == 2
extern void mpu6050_writeWords(struct mpu6050 *self, uint8_t regAddr, uint8_t length, uint16_t* data);
extern void mpu6050_setMemoryBank(struct mpu6050 *self, uint8_t bank, uint8_t prefetchEnabled, uint8_t userBank);
extern void mpu6050_setMemoryStartAddress(struct mpu6050 *self, uint8_t address);
extern void mpu6050_readMemoryBlock(struct mpu6050 *self, uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address);
extern uint8_t mpu6050_writeMemoryBlock(struct mpu6050 *self, const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, uint8_t verify, uint8_t useProgMem);
extern uint8_t mpu6050_writeDMPConfigurationSet(struct mpu6050 *self, const uint8_t *data, uint16_t dataSize, uint8_t useProgMem);
extern uint16_t mpu6050_getFIFOCount(struct mpu6050 *self);
extern void mpu6050_getFIFOBytes(struct mpu6050 *self, uint8_t *data, uint8_t length);
extern uint8_t mpu6050_getIntStatus(struct mpu6050 *self);
extern void mpu6050_resetFIFO(struct mpu6050 *self);
extern int8_t mpu6050_getXGyroOffset(struct mpu6050 *self);
extern void mpu6050_setXGyroOffset(struct mpu6050 *self, int8_t offset);
extern int8_t mpu6050_getYGyroOffset(struct mpu6050 *self);
extern void mpu6050_setYGyroOffset(struct mpu6050 *self, int8_t offset);
extern int8_t mpu6050_getZGyroOffset(struct mpu6050 *self);
extern void mpu6050_setZGyroOffset(struct mpu6050 *self, int8_t offset);
//base dmp
extern uint8_t mpu6050_dmpInitialize(struct mpu6050 *self);
extern void mpu6050_dmpEnable(struct mpu6050 *self);
extern void mpu6050_dmpDisable(struct mpu6050 *self);
extern void mpu6050_getQuaternion(struct mpu6050 *self, const uint8_t* packet, double *qw, double *qx, double *qy, double *qz);
extern void mpu6050_getRollPitchYaw(struct mpu6050 *self, double qw, double qx, double qy, double qz, double *roll, double *pitch, double *yaw);
extern uint8_t mpu6050_getQuaternionWait(struct mpu6050 *self, double *qw, double *qx, double *qy, double *qz);
#endif

#ifdef __cplusplus
}
#endif

#endif
