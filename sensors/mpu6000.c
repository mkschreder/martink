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

///////////////////////////////////////////////////////////////////////////////


#include <stdlib.h>
#include <string.h>
#include <arch/soc.h>
#include "mpu6000.h"

#define MPU6000_AXOFFSET 0
#define MPU6000_AYOFFSET 0
#define MPU6000_AZOFFSET 0
#define MPU6000_AXGAIN 16384.0
#define MPU6000_AYGAIN 16384.0
#define MPU6000_AZGAIN 16384.0
#define MPU6000_GXOFFSET 0
#define MPU6000_GYOFFSET 0
#define MPU6000_GZOFFSET 0
// gain is maxint16 (32768) /range - range degrees of precision
#define MPU6000_GXGAIN 16.4
#define MPU6000_GYGAIN 16.4
#define MPU6000_GZGAIN 16.4

#define MPU6000_SMPLRT_DIV	    	0x19
#define MPU6000_GYRO_CONFIG	    	0x1B
#define MPU6000_ACCEL_CONFIG  		0x1C
#define MPU6000_FIFO_EN		    		0x23
#define MPU6000_INT_PIN_CFG	    	0x37
#define MPU6000_INT_ENABLE	    	0x38
#define MPU6000_INT_STATUS	    	0x3A
#define MPU6000_ACCEL_XOUT_H 			0x3B
#define MPU6000_ACCEL_XOUT_L 			0x3C
#define MPU6000_ACCEL_YOUT_H 			0x3D
#define MPU6000_ACCEL_YOUT_L 			0x3E
#define MPU6000_ACCEL_ZOUT_H 			0x3F
#define MPU6000_ACCEL_ZOUT_L    	0x40
#define MPU6000_TEMP_OUT_H	    	0x41
#define MPU6000_TEMP_OUT_L	    	0x42
#define MPU6000_GYRO_XOUT_H	    	0x43
#define MPU6000_GYRO_XOUT_L	    	0x44
#define MPU6000_GYRO_YOUT_H	    	0x45
#define MPU6000_GYRO_YOUT_L	     	0x46
#define MPU6000_GYRO_ZOUT_H	    	0x47
#define MPU6000_GYRO_ZOUT_L	    	0x48
#define MPU6000_USER_CTRL	    		0x6A
#define MPU6000_PWR_MGMT_1	    	0x6B
#define MPU6000_PWR_MGMT_2	    	0x6C
#define MPU6000_FIFO_COUNTH	    	0x72
#define MPU6000_FIFO_COUNTL	    	0x73
#define MPU6000_FIFO_R_W		   		0x74
#define MPU6000_WHOAMI		    		0x75

// Bits

#define BIT_SLEEP										0x40
#define BIT_H_RESET									0x80
#define BITS_CLKSEL									0x07
#define MPU_CLK_SEL_PLLGYROX				0x01
#define MPU_CLK_SEL_PLLGYROZ				0x03
#define MPU_EXT_SYNC_GYROX					0x02
#define BITS_FS_250DPS              0x00
#define BITS_FS_500DPS              0x08
#define BITS_FS_1000DPS             0x10
#define BITS_FS_2000DPS             0x18
#define BITS_FS_2G                  0x00
#define BITS_FS_4G                  0x08
#define BITS_FS_8G                  0x10
#define BITS_FS_16G                 0x18
#define BITS_FS_MASK                0x18
#define BITS_DLPF_CFG_20HZ          0x04
#define BITS_DLPF_CFG_10HZ          0x05
#define BITS_DLPF_CFG_5HZ           0x06
#define BITS_DLPF_CFG_2100HZ_NOLPF  0x07
#define BITS_DLPF_CFG_MASK          0x07
#define BIT_INT_ANYRD_2CLEAR        0x10
#define BIT_RAW_RDY_EN			    0x01
#define BIT_I2C_IF_DIS              0x10
#define BIT_INT_STATUS_DATA		    0x01
/*
///////////////////////////////////////

float accelOneG = 9.8065;

int32_t accelSum100Hz[3] = { 0, 0, 0 };

int32_t accelSum500Hz[3] = { 0, 0, 0 };

int32_t accelSummedSamples100Hz[3];

int32_t accelSummedSamples500Hz[3];

float accelTCBias[3] = { 0.0f, 0.0f, 0.0f };

int16andUint8_t rawAccel[3];

float nonRotatedAccelData[3];

///////////////////////////////////////

float gyroRTBias[3];

int32_t gyroSum500Hz[3] = { 0, 0, 0 };

int32_t gyroSummedSamples500Hz[3];

float gyroTCBias[3];

int16andUint8_t rawGyro[3];

float nonRotatedGyroData[3];

///////////////////////////////////////

uint8_t accelCalibrating = false;

uint8_t mpu6000Calibrating = false;

float   mpu6000Temperature;

int16andUint8_t rawMPU6000Temperature;
*/

#define MPU6000_CS_Hi gpio_set(self->cs_pin)
#define MPU6000_CS_Lo gpio_clear(self->cs_pin)

static void mpu6000_write_reg(struct mpu6000 *self, uint8_t reg, uint8_t data){
	MPU6000_CS_Lo; 
	serial_putc(self->port, reg); 
	serial_putc(self->port, data); 
	MPU6000_CS_Hi; 
}

static void mpu6000_readBytes(struct mpu6000 *self, uint8_t start, uint8_t count, uint8_t *data){
	MPU6000_CS_Lo; 
	serial_putc(self->port, start | 0x80); 
	for(int c = 0; c < count; c++){
		uint16_t res = serial_putc(self->port, 0x00); 
		if(res != SERIAL_NO_DATA)
			*data++ = res; 
	}
	MPU6000_CS_Hi; 
}

void mpu6000_init(struct mpu6000 *self, serial_dev_t port, pio_dev_t gpio, gpio_pin_t cs_pin){
	self->port = port; 
	self->gpio = gpio; 
	self->cs_pin = cs_pin; 
	
	gpio_configure(cs_pin, GP_OUTPUT); 
	
	mpu6000_write_reg(self, MPU6000_PWR_MGMT_1, BIT_H_RESET);          // Device Reset
  
	delay_ms(150);

	struct {
		uint8_t reg, data; 
	} settings[] = {
		{MPU6000_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ},
		{MPU6000_USER_CTRL, BIT_I2C_IF_DIS},
		{MPU6000_PWR_MGMT_2, 0},
		{MPU6000_SMPLRT_DIV, 0},	// Accel Sample Rate 1000 Hz, Gyro Sample Rate 8000 Hz
		//{MPU6000_CONFIG, dlpfsetting}, 
		{MPU6000_ACCEL_CONFIG, BITS_FS_2G}, 
		{MPU6000_GYRO_CONFIG, BITS_FS_2000DPS}
	}; 
	
	for(unsigned c = 0; c < sizeof(settings) / sizeof(settings[0]); c++){
		mpu6000_write_reg(self, settings[c].reg, settings[c].data); 
		delay_us(1); 
	}
	
	delay_ms(100); 
}


void mpu6000_readRawAcc(struct mpu6000 *self, int16_t* ax, int16_t* ay, int16_t* az){
	uint8_t buffer[16]; 
	mpu6000_readBytes(self, MPU6000_ACCEL_XOUT_H, 6, (uint8_t *)buffer);

	*ax = (((int16_t)buffer[0]) << 8) | buffer[1];
	*ay = (((int16_t)buffer[2]) << 8) | buffer[3];
	*az = (((int16_t)buffer[4]) << 8) | buffer[5];
}

void mpu6000_readRawGyr(struct mpu6000 *self, int16_t* gx, int16_t* gy, int16_t* gz){
	uint8_t buffer[16]; 
	mpu6000_readBytes(self, MPU6000_GYRO_XOUT_H, 6, (uint8_t *)buffer);

	*gx = (((int16_t)buffer[0]) << 8) | buffer[1];
	*gy = (((int16_t)buffer[2]) << 8) | buffer[3];
	*gz = (((int16_t)buffer[4]) << 8) | buffer[5];
}

void mpu6000_convertAcc(struct mpu6000 *self, int16_t ax, int16_t ay, int16_t az, float *axg, float *ayg, float *azg){
	(void)(self); 
	*axg = (float)(ax-MPU6000_AXOFFSET)/MPU6000_AXGAIN;
	*ayg = (float)(ay-MPU6000_AYOFFSET)/MPU6000_AYGAIN;
	*azg = (float)(az-MPU6000_AZOFFSET)/MPU6000_AZGAIN;
}

void mpu6000_convertGyr(struct mpu6000 *self, int16_t gx, int16_t gy, int16_t gz, float *gxd, float *gyd, float *gzd){
	(void)(self); 
	*gxd = (float)(gx-MPU6000_GXOFFSET)/MPU6000_GXGAIN;
	*gyd = (float)(gy-MPU6000_GYOFFSET)/MPU6000_GYGAIN;
	*gzd = (float)(gz-MPU6000_GZOFFSET)/MPU6000_GZGAIN;
}
/*
void mpu6000_getRawData(struct mpu6000 *self, int16_t* ax, int16_t* ay, int16_t* az, int16_t* gx, int16_t* gy, int16_t* gz) {
	uint8_t buffer[16]; 
	
	mpu6000_readBytes(self, MPU6000_ACCEL_XOUT_H, 14, (uint8_t *)buffer);

	*ax = (((int16_t)buffer[0]) << 8) | buffer[1];
	*ay = (((int16_t)buffer[2]) << 8) | buffer[3];
	*az = (((int16_t)buffer[4]) << 8) | buffer[5];
	// 6 and 7 is temp
	*gx = (((int16_t)buffer[8]) << 8) | buffer[9];
	*gy = (((int16_t)buffer[10]) << 8) | buffer[11];
	*gz = (((int16_t)buffer[12]) << 8) | buffer[13];
}


void mpu6000_convertData(struct mpu6000 *self, 
	int16_t ax, int16_t ay, int16_t az, 
	int16_t gx, int16_t gy, int16_t gz, 
	float *axg, float *ayg, float *azg, 
	float *gxd, float *gyd, float *gzd
){
	(void)(self); 
	*axg = (float)(ax-MPU6000_AXOFFSET)/MPU6000_AXGAIN;
	*ayg = (float)(ay-MPU6000_AYOFFSET)/MPU6000_AYGAIN;
	*azg = (float)(az-MPU6000_AZOFFSET)/MPU6000_AZGAIN;
	*gxd = (float)(gx-MPU6000_GXOFFSET)/MPU6000_GXGAIN;
	*gyd = (float)(gy-MPU6000_GYOFFSET)/MPU6000_GYGAIN;
	*gzd = (float)(gz-MPU6000_GZOFFSET)/MPU6000_GZGAIN;
}
*/
/*
void readMPU6000(void)
{
    ENABLE_MPU6000;
    
    spiTransfer(MPU6000_SPI, MPU6000_ACCEL_XOUT_H | 0x80);

    rawAccel[XAXIS].bytes[1]       = spiTransfer(MPU6000_SPI, 0x00);
    rawAccel[XAXIS].bytes[0]       = spiTransfer(MPU6000_SPI, 0x00);
    rawAccel[YAXIS].bytes[1]       = spiTransfer(MPU6000_SPI, 0x00);
    rawAccel[YAXIS].bytes[0]       = spiTransfer(MPU6000_SPI, 0x00);
    rawAccel[ZAXIS].bytes[1]       = spiTransfer(MPU6000_SPI, 0x00);
    rawAccel[ZAXIS].bytes[0]       = spiTransfer(MPU6000_SPI, 0x00);

    rawMPU6000Temperature.bytes[1] = spiTransfer(MPU6000_SPI, 0x00);
    rawMPU6000Temperature.bytes[0] = spiTransfer(MPU6000_SPI, 0x00);

    rawGyro[ROLL ].bytes[1]        = spiTransfer(MPU6000_SPI, 0x00);
    rawGyro[ROLL ].bytes[0]        = spiTransfer(MPU6000_SPI, 0x00);
    rawGyro[PITCH].bytes[1]        = spiTransfer(MPU6000_SPI, 0x00);
    rawGyro[PITCH].bytes[0]        = spiTransfer(MPU6000_SPI, 0x00);
    rawGyro[YAW  ].bytes[1]        = spiTransfer(MPU6000_SPI, 0x00);
    rawGyro[YAW  ].bytes[0]        = spiTransfer(MPU6000_SPI, 0x00);

    DISABLE_MPU6000;
}

void computeMPU6000RTData(void)
{
    uint8_t  axis;
    uint16_t samples;

    float accelSum[3]    = { 0.0f, 0.0f, 0.0f };
    float gyroSum[3]     = { 0.0f, 0.0f, 0.0f };

    mpu6000Calibrating = true;

    for (samples = 0; samples < 5000; samples++)
    {
        readMPU6000();

        computeMPU6000TCBias();

       	accelSum[XAXIS] += ((float)rawAccel[XAXIS].value - eepromConfig.accelBiasMPU[XAXIS] - accelTCBias[XAXIS]) * eepromConfig.accelScaleFactorMPU[XAXIS];
       	accelSum[YAXIS] += ((float)rawAccel[YAXIS].value - eepromConfig.accelBiasMPU[YAXIS] - accelTCBias[YAXIS]) * eepromConfig.accelScaleFactorMPU[YAXIS];
       	accelSum[ZAXIS] += ((float)rawAccel[ZAXIS].value - eepromConfig.accelBiasMPU[ZAXIS] - accelTCBias[ZAXIS]) * eepromConfig.accelScaleFactorMPU[ZAXIS];

        gyroSum[ROLL ]  += (float)rawGyro[ROLL ].value  - gyroTCBias[ROLL ];
        gyroSum[PITCH]  += (float)rawGyro[PITCH].value  - gyroTCBias[PITCH];
        gyroSum[YAW  ]  += (float)rawGyro[YAW  ].value  - gyroTCBias[YAW  ];

        delayMicroseconds(1000);
    }

    for (axis = 0; axis < 3; axis++)
    {
        accelSum[axis]   = accelSum[axis] / 5000.0f;
        gyroRTBias[axis] = gyroSum[axis]  / 5000.0f;
    }

    accelOneG = sqrt(SQR(accelSum[XAXIS]) + SQR(accelSum[YAXIS]) + SQR(accelSum[ZAXIS]));

    mpu6000Calibrating = false;
}

///////////////////////////////////////////////////////////////////////////////
// Compute MPU6000 Temperature Compensation Bias
///////////////////////////////////////////////////////////////////////////////

void computeMPU6000TCBias(void)
{
    mpu6000Temperature = (float) (rawMPU6000Temperature.value) / 340.0f + 35.0f;

    accelTCBias[XAXIS] = eepromConfig.accelTCBiasSlope[XAXIS] * mpu6000Temperature + eepromConfig.accelTCBiasIntercept[XAXIS];
    accelTCBias[YAXIS] = eepromConfig.accelTCBiasSlope[YAXIS] * mpu6000Temperature + eepromConfig.accelTCBiasIntercept[YAXIS];
    accelTCBias[ZAXIS] = eepromConfig.accelTCBiasSlope[ZAXIS] * mpu6000Temperature + eepromConfig.accelTCBiasIntercept[ZAXIS];

    gyroTCBias[ROLL ]  = eepromConfig.gyroTCBiasSlope[ROLL ]  * mpu6000Temperature + eepromConfig.gyroTCBiasIntercept[ROLL ];
    gyroTCBias[PITCH]  = eepromConfig.gyroTCBiasSlope[PITCH]  * mpu6000Temperature + eepromConfig.gyroTCBiasIntercept[PITCH];
    gyroTCBias[YAW  ]  = eepromConfig.gyroTCBiasSlope[YAW  ]  * mpu6000Temperature + eepromConfig.gyroTCBiasIntercept[YAW  ];
}

*/
