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
#define HMC5883L_SCALE HMC5883L_SCALE13

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

#define HMC5883L_STATUS_READY 1

#define TIMEOUT(t) do { self->time = timestamp_from_now_us(t); \
		PT_WAIT_UNTIL(pt, timestamp_expired(self->time)); } while(0); 

#define STORE_VEC3_16(target) target[0] = READ_INT16(self->buf); target[1] = READ_INT16(self->buf + 2); target[2] = READ_INT16(self->buf + 4);

PT_THREAD(_hmc5883l_thread(struct libk_thread *kthread, struct pt *pt)); 
PT_THREAD(_hmc5883l_thread(struct libk_thread *kthread, struct pt *pt)){
	struct hmc5883l *self = container_of(kthread, struct hmc5883l, thread); 
	
	PT_BEGIN(pt); 
	
	// wait for the compass to start
	TIMEOUT(50000L); 
	
	IO_BEGIN(pt, &self->tr, self->dev); 
	
	self->buf[0] = HMC5883L_NUM_SAMPLES4 | HMC5883L_RATE30; 
	IO_WRITE(pt, &self->tr, self->dev, HMC5883L_CONFREGA, self->buf, 1);
	self->buf[0] = HMC5883L_SCALE << 5; 
	IO_WRITE(pt, &self->tr, self->dev, HMC5883L_CONFREGB, self->buf, 1);
	self->buf[0] = HMC5883L_MEASUREMODE; 
	IO_WRITE(pt, &self->tr, self->dev, HMC5883L_MODEREG, self->buf, 1);
	
	IO_READ(pt, &self->tr, self->dev, HMC5883L_REG_IDA, self->buf, 3); 
	self->sensor_id = READ_INT24(self->buf); 
	
	IO_END(pt, &self->tr, self->dev); 
	
	TIMEOUT(7000L); 
	
	self->status |= HMC5883L_STATUS_READY; 
	
	while(1){
		IO_BEGIN(pt, &self->tr, self->dev); 
		
		IO_READ(pt, &self->tr, self->dev, HMC5883L_DATAREGBEGIN, self->buf, 6); 
		STORE_VEC3_16(self->raw_mag); 
		
		IO_END(pt, &self->tr, self->dev); 
	
		static timestamp_t tfps = 0; 
		static int fps = 0; 
		if(timestamp_expired(tfps)){
			printf("HMC FPS: %d\n", fps); 
			fps = 0; 
			tfps = timestamp_from_now_us(1000000); 
		} fps++; 
		
		//TIMEOUT(10000); 
		
		PT_YIELD(pt); 
		//printf("HMC: %d %d %d\n", self->raw_mag[0], self->raw_mag[1], self->raw_mag[2]); 
		
		//TIMEOUT(10000L);
	}
	
	PT_END(pt); 
}

void hmc5883l_init(struct hmc5883l *self, block_dev_t i2c) {
	self->dev = i2c;
	self->status = 0; 
	
	blk_transfer_init(&self->tr); 
	
	libk_create_thread(&self->thread, _hmc5883l_thread, "hmc5883l"); 
	
	switch(HMC5883L_SCALE) {
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
}

void hmc5883l_readRawMag(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw) {
	*mxraw = self->raw_mag[0];
	*mzraw = self->raw_mag[1];
	*myraw = self->raw_mag[2];
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
