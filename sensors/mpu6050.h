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
	void (*getConvAcc)(double *x, double *y, double *z); 
	void (*getConvGyr)(double *x, double *y, double *z); 
}; 

extern const struct mpu6050 mpu6050;

//definitions
#define MPU6050_ADDR (0x68 <<1) //device address - 0x68 pin low (GND), 0x69 pin high (VCC)
#define MPU6050_GETATTITUDE 0

//functions
extern void mpu6050_init(void);
extern uint8_t mpu6050_testConnection(void);

#if MPU6050_GETATTITUDE == 0
extern void mpu6050_getRawData(int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz);
extern void mpu6050_getConvAcc(double* axg, double* ayg, double* azg);
extern void mpu6050_getConvGyr(double* gxds, double* gyds, double* gzds);
#endif

extern void mpu6050_setSleepDisabled(void);
extern void mpu6050_setSleepEnabled(void);

extern int8_t mpu6050_readBytes(uint8_t regAddr, uint8_t length, uint8_t *data);
extern int8_t mpu6050_readByte(uint8_t regAddr, uint8_t *data);
extern void mpu6050_writeBytes(uint8_t regAddr, uint8_t length, uint8_t* data);
extern void mpu6050_writeByte(uint8_t regAddr, uint8_t data);
extern int8_t mpu6050_readBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data);
extern int8_t mpu6050_readBit(uint8_t regAddr, uint8_t bitNum, uint8_t *data);
extern void mpu6050_writeBits(uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data);
extern void mpu6050_writeBit(uint8_t regAddr, uint8_t bitNum, uint8_t data);

#if MPU6050_GETATTITUDE == 1
extern void mpu6050_updateQuaternion(void);
extern void mpu6050_getQuaternion(double *qw, double *qx, double *qy, double *qz);
extern void mpu6050_getRollPitchYaw(double *pitch, double *roll, double *yaw);
#endif

#if MPU6050_GETATTITUDE == 2
extern void mpu6050_writeWords(uint8_t regAddr, uint8_t length, uint16_t* data);
extern void mpu6050_setMemoryBank(uint8_t bank, uint8_t prefetchEnabled, uint8_t userBank);
extern void mpu6050_setMemoryStartAddress(uint8_t address);
extern void mpu6050_readMemoryBlock(uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address);
extern uint8_t mpu6050_writeMemoryBlock(const uint8_t *data, uint16_t dataSize, uint8_t bank, uint8_t address, uint8_t verify, uint8_t useProgMem);
extern uint8_t mpu6050_writeDMPConfigurationSet(const uint8_t *data, uint16_t dataSize, uint8_t useProgMem);
extern uint16_t mpu6050_getFIFOCount(void);
extern void mpu6050_getFIFOBytes(uint8_t *data, uint8_t length);
extern uint8_t mpu6050_getIntStatus(void);
extern void mpu6050_resetFIFO();
extern int8_t mpu6050_getXGyroOffset(void);
extern void mpu6050_setXGyroOffset(int8_t offset);
extern int8_t mpu6050_getYGyroOffset(void);
extern void mpu6050_setYGyroOffset(int8_t offset);
extern int8_t mpu6050_getZGyroOffset(void);
extern void mpu6050_setZGyroOffset(int8_t offset);
//base dmp
extern uint8_t mpu6050_dmpInitialize(void);
extern void mpu6050_dmpEnable(void);
extern void mpu6050_dmpDisable(void);
extern void mpu6050_getQuaternion(const uint8_t* packet, double *qw, double *qx, double *qy, double *qz);
extern void mpu6050_getRollPitchYaw(double qw, double qx, double qy, double qz, double *roll, double *pitch, double *yaw);
extern uint8_t mpu6050_getQuaternionWait(double *qw, double *qx, double *qy, double *qz);
#endif

#ifdef __cplusplus
}
#endif

#endif
