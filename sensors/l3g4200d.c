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
	* Davide Gironi, original implementation
*/

#include <stdlib.h>

#include <arch/soc.h>

#include "l3g4200d.h"


//set range
#define L3G4200D_RANGE250 0
#define L3G4200D_RANGE500 1
#define L3G4200D_RANGE2000 2
#define L3G4200D_RANGE L3G4200D_RANGE2000

//define DPS conversion
#if L3G4200D_RANGE == L3G4200D_RANGE250
#define L3G4200D_GAIN 0.00875
#elif L3G4200D_RANGE == L3G4200D_RANGE500
#define L3G4200D_GAIN 0.0175
#elif L3G4200D_RANGE == L3G4200D_RANGE2000
#define L3G4200D_GAIN 0.07
#endif

#define L3G4200D_CALIBRATED 0 //set to 1 if calibrated
#define L3G4200D_CALIBRATEDDOTEMPCOMP 0 //set to 1 to enable temperature compensation

#if L3G4200D_CALIBRATED == 1
//calibration values
#define L3G4200D_OFFSETX -17.04
#define L3G4200D_OFFSETY -6.56
#define L3G4200D_OFFSETZ -20.50
#define L3G4200D_GAINX 0.068
#define L3G4200D_GAINY 0.073
#define L3G4200D_GAINZ 0.069
#if L3G4200D_CALIBRATEDDOTEMPCOMP == 1
#define L3G4200D_TEMPCOMPX -0.002725
#define L3G4200D_TEMPCOMPY 0.000421
#define L3G4200D_TEMPCOMPZ 0.005629
#endif
#endif

//registers
#define L3G4200D_WHO_AM_I      0x0F

#define L3G4200D_CTRL_REG1     0x20
#define L3G4200D_CTRL_REG2     0x21
#define L3G4200D_CTRL_REG3     0x22
#define L3G4200D_CTRL_REG4     0x23
#define L3G4200D_CTRL_REG5     0x24
#define L3G4200D_REFERENCE     0x25
#define L3G4200D_OUT_TEMP      0x26
#define L3G4200D_STATUS_REG    0x27

#define L3G4200D_OUT_X_L       0x28
#define L3G4200D_OUT_X_H       0x29
#define L3G4200D_OUT_Y_L       0x2A
#define L3G4200D_OUT_Y_H       0x2B
#define L3G4200D_OUT_Z_L       0x2C
#define L3G4200D_OUT_Z_H       0x2D

#define L3G4200D_FIFO_CTRL_REG 0x2E
#define L3G4200D_FIFO_SRC_REG  0x2F

#define L3G4200D_INT1_CFG      0x30
#define L3G4200D_INT1_SRC      0x31
#define L3G4200D_INT1_THS_XH   0x32
#define L3G4200D_INT1_THS_XL   0x33
#define L3G4200D_INT1_THS_YH   0x34
#define L3G4200D_INT1_THS_YL   0x35
#define L3G4200D_INT1_THS_ZH   0x36
#define L3G4200D_INT1_THS_ZL   0x37
#define L3G4200D_INT1_DURATION 0x38

//reference temperature
/*#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
float l3g4200d_gtemp = 0; //temperature used for compensation
#endif*/

/*
 * set reference temperature
 */
void l3g4200d_settemperatureref(struct l3g4200d *self) {
	uint8_t data = L3G4200D_OUT_TEMP;
	i2c_start_write(self->i2c, self->addr, &data, 1);
	i2c_start_read(self->i2c, self->addr, &data, 1);
	i2c_stop(self->i2c); 
	self->temperatureref = (int8_t) data;
	//#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
	//l3g4200d_gtemp = (float)rawtemp;
	//#endif
}

/*
 * get temperature variation
 */
int8_t l3g4200d_gettemperaturediff(struct l3g4200d *self) {
	uint8_t data = L3G4200D_OUT_TEMP;
	i2c_start_write(self->i2c, self->addr, &data, 1);
	i2c_start_read(self->i2c, self->addr, &data, 1);
	i2c_stop(self->i2c); 
	return self->temperatureref - (int8_t)data;
}

/*
 * set offset variables
 */
void l3g4200d_setoffset(struct l3g4200d *self, float offsetx, float offsety, float offsetz) {
	self->offsetx = offsetx;
	self->offsety = offsety;
	self->offsetz = offsetz;
}

/*
 * get raw data
 */
void l3g4200d_read_raw(struct l3g4200d *self, int16_t *gxraw, int16_t *gyraw, int16_t *gzraw) {
	uint8_t buff[6];
	uint8_t reg = L3G4200D_OUT_X_L | (1 << 7);
	i2c_start_write(self->i2c, self->addr, &reg, 1);
	i2c_start_read(self->i2c, self->addr, buff, 6);
	i2c_stop(self->i2c); 
	
	*gxraw = ((buff[1] << 8) | buff[0]);
	*gyraw = ((buff[3] << 8) | buff[2]);
	*gzraw = ((buff[5] << 8) | buff[4]);
}


/*
 * get converted data deg/sec
 */
void l3g4200d_read_converted(struct l3g4200d *self, float* gx, float* gy, float* gz) {
	int16_t gxraw = 0;
	int16_t gyraw = 0;
	int16_t gzraw = 0;
	l3g4200d_read_raw(self, &gxraw, &gyraw, &gzraw);

	//#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
	//l3g4200d_gtemp = l3g4200d_gtemp*0.95 + 0.05*l3g4200d_gettemperaturediff(); //filtered temperature compansation
	//#endif

	/*#if L3G4200D_CALIBRATED == 1
		#if L3G4200D_CALIBRATEDDOTEMPCOMP == 1
	*gx = (gxraw - (float)((L3G4200D_TEMPCOMPX*l3g4200d_gtemp) + (float)self->offsetx)) * (float)L3G4200D_GAINX;
	*gy = (gyraw - (float)((L3G4200D_TEMPCOMPY*l3g4200d_gtemp) + (float)self->offsety)) * (float)L3G4200D_GAINY;
	*gz = (gzraw - (float)((L3G4200D_TEMPCOMPZ*l3g4200d_gtemp) + (float)self->offsetz)) * (float)L3G4200D_GAINZ;
		#else
	*gx = (gxraw-(float)self->offsetx) * (float)L3G4200D_GAINX;
	*gy = (gyraw-(float)self->offsety) * (float)L3G4200D_GAINY;
	*gz = (gzraw-(float)self->offsetz) * (float)L3G4200D_GAINZ;
		#endif
	#else*/
	*gx = (gxraw-(float)self->offsetx) * (float)L3G4200D_GAIN;
	*gy = (gyraw-(float)self->offsety) * (float)L3G4200D_GAIN;
	*gz = (gzraw-(float)self->offsetz) * (float)L3G4200D_GAIN;
	//#endif
}

/*
 * init L3G4200D_
 */
void l3g4200d_init(struct l3g4200d *self, i2c_dev_t i2c, uint8_t addr) {
	self->i2c = i2c; 
	self->addr = addr; 
	
	uint8_t reg[] = {L3G4200D_CTRL_REG1, 0};
	reg[1] = 0x0f; // normal power mode, all axes
	i2c_start_write(self->i2c, self->addr, reg, 2);
	i2c_stop(self->i2c); 

	
	reg[1] = L3G4200D_CTRL_REG4; reg[1] = L3G4200D_RANGE<<4; // normal power mode, all axes
	i2c_start_write(self->i2c, self->addr, reg, 2);
	i2c_stop(self->i2c);
	/*
	#if L3G4200D_CALIBRATED == 1
	//init offset
	self->offsetx = L3G4200D_OFFSETX;
	self->offsety = L3G4200D_OFFSETY;
	self->offsetz = L3G4200D_OFFSETZ;
	#endif
*/
	l3g4200d_settemperatureref(self);
}
