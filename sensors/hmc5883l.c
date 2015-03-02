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

#pragma message("skipping hmc5883 driver")
#if 0
static PT_THREAD(_hmc5883l_init_thread(struct pt *thr, struct hmc5883l *self)){
	struct pt *bthr = &self->bthread; 
	
	PT_BEGIN(thr); 
	
	// ensure only run when not ready
	PT_WAIT_WHILE(thr, self->status & HMC5883L_STATUS_READY); 
	
	// wait for the compass to start
	self->time = timestamp_from_now_us(50000L); 
	PT_WAIT_UNTIL(thr, timestamp_expired(self->time)); 
	
	self->buffer[0] = HMC5883L_CONFREGA; 
	self->buffer[1] = HMC5883L_NUM_SAMPLES4 | HMC5883L_RATE30; 
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2)); 
	
	self->buffer[0] = HMC5883L_CONFREGB; 
	self->buffer[1] = HMC5883L_SCALE << 5; 
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2)); 
	
	//set measurement mode
	self->buffer[0] = HMC5883L_MODEREG; 
	self->buffer[1] = HMC5883L_MEASUREMODE; 
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2)); 
	
	// read id 
	PT_SPAWN(thr, bthr, i2c_read_reg_thread_sp(self->i2c, bthr, self->addr, HMC5883L_REG_IDA, self->buffer, 3)); 
	self->sensor_id = ((uint32_t)self->buffer[0] << 16) | ((uint32_t)self->buffer[1] << 8) | self->buffer[2]; 
	
  self->time = timestamp_from_now_us(7000L); 
	PT_WAIT_UNTIL(thr, timestamp_expired(self->time)); 
	
	self->status |= HMC5883L_STATUS_READY; 
	
	PT_END(thr); 
}

static PT_THREAD(_hmc5883l_update_thread(struct pt *thr, struct hmc5883l *self)){
	struct pt *bthr = &self->bthread; 
	
	PT_BEGIN(thr); 
	
	while(1){
		PT_SPAWN(thr, bthr, i2c_read_reg_thread_sp(self->i2c, bthr, 
			self->addr, HMC5883L_DATAREGBEGIN, self->buffer, 6)); 
		
		uint8_t *buff = self->buffer; 
		
		self->raw_mx = (int16_t)((buff[0] << 8) | buff[1]);
		self->raw_my = (int16_t)((buff[2] << 8) | buff[3]);
		self->raw_mz = (int16_t)((buff[4] << 8) | buff[5]);
		
		// sleep for 5ms
		self->time = timestamp_from_now_us(5000L); 
		PT_WAIT_UNTIL(thr, timestamp_expired(self->time)); 
	}
	
	PT_END(thr); 
}

void hmc5883l_init(struct hmc5883l *self, i2c_dev_t i2c, uint8_t addr) {
	self->i2c = i2c;
	self->addr = addr;
	PT_INIT(&self->uthread); 
	PT_INIT(&self->ithread); 
	PT_INIT(&self->bthread); 
	self->status = 0; 
	
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

void hmc5883l_update(struct hmc5883l *self){
	if(self->status & HMC5883L_STATUS_READY)
		_hmc5883l_update_thread(&self->uthread, self); 
	else
		_hmc5883l_init_thread(&self->ithread, self); 
}

void hmc5883l_readRawMag(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw) {
	*mxraw = self->raw_mx;
	*mzraw = self->raw_my;
	*myraw = self->raw_mz;
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
#endif
