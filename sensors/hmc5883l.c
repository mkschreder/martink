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
*/

#include <stdlib.h>
#include <string.h>

#include <arch/soc.h>
#include "hmc5883l.h"


//registers
#define HMC5883L_CONFREGA 	0
#define HMC5883L_CONFREGB 	1
#define HMC5883L_MODEREG 		2
#define HMC5883L_DATAREGBEGIN 3
#define HMC5883L_REG_STATUS 9
#define HMC5883L_REG_IDA 		10
#define HMC5883L_REG_IDB 		11
#define HMC5883L_REG_IDC 		12

//setup measurement mode
#define HMC5883L_MEASURECONTINOUS 0x00
#define HMC5883L_MEASURESINGLESHOT 0x01
#define HMC5883L_MEASUREIDLE 0x03
#define HMC5883L_MEASUREMODE HMC5883L_MEASURECONTINOUS

#define HMC5883L_NUM_SAMPLES1 (0 << 5)
#define HMC5883L_NUM_SAMPLES2 (1 << 5)
#define HMC5883L_NUM_SAMPLES4 (2 << 5)
#define HMC5883L_NUM_SAMPLES8 (3 << 5)

#define HMC5883L_RATE0_75 (0 << 2)
#define HMC5883L_RATE1_5 (1 << 2)
#define HMC5883L_RATE3_0 (2 << 2)
#define HMC5883L_RATE7_5 (3 << 2)
#define HMC5883L_RATE15 (4 << 2)
#define HMC5883L_RATE30 (5 << 2)
#define HMC5883L_RATE75 (6 << 2)

//setup scale
#define HMC5883L_SCALE088 0 //0.88
#define HMC5883L_SCALE13 1 //1.3
#define HMC5883L_SCALE19 2 //1.9
#define HMC5883L_SCALE25 3 //2.5
#define HMC5883L_SCALE40 4 //4.0
#define HMC5883L_SCALE47 5 //4.7
#define HMC5883L_SCALE56 6 //5.6
#define HMC5883L_SCALE81 7 //8.1
#define HMC5883L_SCALE HMC5883L_SCALE88

#define HMC5883L_CALIBRATED 1 //enable this if this magn is calibrated

//calibration values
#if HMC5883L_CALIBRATED == 1
#define HMC5883L_OFFSETX -72
#define HMC5883L_OFFSETY -43
#define HMC5883L_OFFSETZ 460
#define HMC5883L_GAINX1 0.952017
#define HMC5883L_GAINX2 0.00195895
#define HMC5883L_GAINX3 0.0139661
#define HMC5883L_GAINY1 0.00195895
#define HMC5883L_GAINY2 0.882824
#define HMC5883L_GAINY3 0.00760243
#define HMC5883L_GAINZ1 0.0139661
#define HMC5883L_GAINZ2 0.00760243
#define HMC5883L_GAINZ3 0.995365
#endif

/*
static uint8_t hmc5883l_read_reg(struct hmc5883l *self, uint8_t reg){
	uint8_t res = 0; 
	i2c_start_write(self->i2c, self->addr, &reg, 1);
	//delay_us(5000); 
	i2c_stop(self->i2c);
	i2c_start_read(self->i2c, self->addr, &res, 1);
	//delay_us(5000); 
	i2c_stop(self->i2c); 
	return res; 
}*/
/*
 * init the hmc5883l
 */
void hmc5883l_init(struct hmc5883l *self, i2c_dev_t i2c, uint8_t addr) {
	self->i2c = i2c;
	self->addr = addr;
	
	delay_us(50000L); 
	
	//set scale
	//self->scale = 0;
	// todo: move out
	uint8_t scale = HMC5883L_SCALE13; 
	switch(scale) {
		case HMC5883L_SCALE088: 
			self->scale = 0.73;
			break; 
		case HMC5883L_SCALE13: 
			self->scale = 0.92;
			break;
		case HMC5883L_SCALE19: 
			self->scale = 1.22;
			break;
		case HMC5883L_SCALE25: 
			self->scale = 1.52;
			break;
		case HMC5883L_SCALE40: 
			self->scale = 2.27;
			break;
		case HMC5883L_SCALE47: 
			self->scale = 2.56;
			break; 
		case HMC5883L_SCALE56: 
			self->scale = 3.03;
			break;
		case HMC5883L_SCALE81: 
			self->scale = 4.35;
			break;
	}
	
  delay_us(50000L);  //Wait before start
  /*
	uint8_t buf[] = {
		HMC5883L_CONFREGA,
		0x70, //Configuration Register A -- 0 11 100 00 num samples: 8 ; output rate: 15Hz ; normal measurement mode
		HMC5883L_CONFREGB,
		0x20, //Configuration Register B -- 001 00000 configuration gain 1.3Ga
		HMC5883L_MODEREG,
		0x00 //Mode register -- 000000 00 continuous Conversion Mode
	};
	i2c_start_write(i2c, addr, buf, sizeof(buf));
	i2c_stop(i2c);
  */
  uint8_t buf[2]; 
  
	buf[0] = HMC5883L_CONFREGA; 
	buf[1] = HMC5883L_NUM_SAMPLES4 | HMC5883L_RATE30; 
	i2c_start_write(i2c, addr, buf, 2);
	i2c_stop(i2c);
	
	buf[0] = HMC5883L_CONFREGB; 
	buf[1] = scale << 5; 
	i2c_start_write(i2c, addr, buf, 2);
	i2c_stop(i2c);
	
	//set measurement mode
	buf[0] = HMC5883L_MODEREG; 
	buf[1] = HMC5883L_MEASUREMODE; 
	i2c_start_write(i2c, addr, buf, 2);
	i2c_stop(i2c);
	
  delay_us(7000L);
}

uint32_t hmc5883l_read_id(struct hmc5883l *self){
	uint8_t reg = HMC5883L_REG_IDA; 
	uint8_t buf[3] = {0}; /* = {
		hmc5883l_read_reg(self, HMC5883L_REG_IDA), 
		hmc5883l_read_reg(self, HMC5883L_REG_IDB), 
		hmc5883l_read_reg(self, HMC5883L_REG_IDC) 
	}; */
	i2c_start_write(self->i2c, self->addr, &reg, 1);
	if(i2c_stop(self->i2c) != 1) return -1; // sensor needs a stop
	i2c_start_read(self->i2c, self->addr, buf, 3);
	i2c_stop(self->i2c);
	return ((uint32_t)buf[0] << 16) | ((uint32_t)buf[1] << 8) | buf[2]; 
}

void hmc5883l_readRawMag(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw) {
	uint8_t buff[6] = {HMC5883L_DATAREGBEGIN};
	i2c_start_write(self->i2c, self->addr, buff, 1);
	i2c_stop(self->i2c); // sensor needs a stop
	memset(buff, 0, sizeof(buff)); 
	i2c_start_read(self->i2c, self->addr, buff, 6);
	i2c_stop(self->i2c);
	
	*mxraw = (int16_t)((buff[0] << 8) | buff[1]);
	*mzraw = (int16_t)((buff[2] << 8) | buff[3]);
	*myraw = (int16_t)((buff[4] << 8) | buff[5]);
}

void hmc5883l_convertMag(struct hmc5883l *self, 
	int16_t mxraw, int16_t myraw, int16_t mzraw, 
	float *mx, float *my, float *mz){
	*mx = mxraw * self->scale; 
	*my = myraw * self->scale; 
	*mz = mzraw * self->scale; 
	/*
	float mxt = mxraw - HMC5883L_OFFSETX;
	float myt = myraw - HMC5883L_OFFSETY;
	float mzt = mzraw - HMC5883L_OFFSETZ;
	*mx = HMC5883L_GAINX1 * mxt + HMC5883L_GAINX2 * myt + HMC5883L_GAINX3 * mzt;
	*my = HMC5883L_GAINY1 * mxt + HMC5883L_GAINY2 * myt + HMC5883L_GAINY3 * mzt;
	*mz = HMC5883L_GAINZ1 * mxt + HMC5883L_GAINZ2 * myt + HMC5883L_GAINZ3 * mzt;
	*/
}

/*
 * get scaled data
 */
/*
void hmc5883l_read_adjusted(struct hmc5883l *self, float *mx, float *my, float *mz) {
	int16_t mxraw = 0;
	int16_t myraw = 0;
	int16_t mzraw = 0;
	hmc5883l_read_raw(self, &mxraw, &myraw, &mzraw);

	#if HMC5883L_CALIBRATED == 1
	float mxt = mxraw - HMC5883L_OFFSETX;
	float myt = myraw - HMC5883L_OFFSETY;
	float mzt = mzraw - HMC5883L_OFFSETZ;
	*mx = HMC5883L_GAINX1 * mxt + HMC5883L_GAINX2 * myt + HMC5883L_GAINX3 * mzt;
	*my = HMC5883L_GAINY1 * mxt + HMC5883L_GAINY2 * myt + HMC5883L_GAINY3 * mzt;
	*mz = HMC5883L_GAINZ1 * mxt + HMC5883L_GAINZ2 * myt + HMC5883L_GAINZ3 * mzt;
	#else
	
	// *mx = mxraw * self->scale;
	// *my = myraw * self->scale;
	// *mz = mzraw * self->scale;
	*mx = mxraw;
	*my = myraw;
	*mz = mzraw; 
	#endif
}
*/
