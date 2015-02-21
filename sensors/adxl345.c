/*
	Accelerometer driver

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

	Author: Martin K. Schröder (threaded, async)
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
	
	Special thanks to:
	* Davide Gironi, original implementation
*/

#include <stdlib.h>

#include <arch/soc.h>

#include "adxl345.h"
#include <thread/pt.h>


//note: we use only 10 bit resolution

//definitions
#define ADXL345_RANGE2G 0x00 //sensitivity to 2g and measurement mode
#define ADXL345_RANGE4G 0x01 //sensitivity to 4g and measurement mode
#define ADXL345_RANGE8G 0x02 //sensitivity to 8g and measurement mode
#define ADXL345_RANGE16G 0x03 //sensitivity to 16g and measurement mode
#define ADXL345_FULLRANGE 0 //1 to enable 0 to disable

//i2c settings
#define ADXL345_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define ADXL345_I2CINIT 1 //init i2c

//settings
#define ADXL345_RANGE ADXL345_RANGE2G //select from sensitivity above
#define ADXL345_LOWPASSENABLED 1 //1 to enable the low pass filter, 0 to disable

//automatic definitions
//gravity is 1g, pressure on axis is (counts @ 1g - counts @ -1g) / sensitivity g
//let's suppose for a 2g range a sensitivity of 8-bit (256 counts max, from -128 @ -2g, to 128 @ 2g),
//the value @ 1g should be 128/2, and -128/2 @ -1g, so (64 - -64) / 2 = 64
#if ADXL345_RANGE == ADXL345_RANGE2G
#define ADXL345_RANGEVAL 256
#elif ADXL345_RANGE == ADXL345_RANGE4G
#if ADXL345_FULLRANGE == 0
#define ADXL345_RANGEVAL 128
#else
#define ADXL345_RANGEVAL 256
#endif
#elif ADXL345_RANGE == ADXL345_RANGE8G
#if ADXL345_FULLRANGE == 0
#define ADXL345_RANGEVAL 64
#else
#define ADXL345_RANGEVAL 256
#endif
#elif ADXL345_RANGE == ADXL345_RANGE16G
#if ADXL345_FULLRANGE == 0
#define ADXL345_RANGEVAL 32
#else
#define ADXL345_RANGEVAL 256
#endif
#endif

#define ADXL345_CALIBRATED 0 //enable this if this accel is calibrated
//to calibrate the sensor collect values placing the accellerometer on every position of an ideal cube,
//for example on axis-x you can read -62 @ -1g and +68 @ 1g counts, the scale factor will be 130 / 2,
//offset is = 1g - counts/scale factor, with a value of -62 @1g, -62 / (130/2) = 0.95
//offset should be 1-0.95 = 0.05
#if ADXL345_CALIBRATED == 1
#define ADXL345_CALRANGEVALX 0
#define ADXL345_CALRANGEVALY 0
#define ADXL345_CALRANGEVALZ 0
#define ADXL345_CALOFFSETX 0
#define ADXL345_CALOFFSETY 0
#define ADXL345_CALOFFSETZ 0
#endif

#include <math.h>
#include <string.h>

#include <thread/pt.h>

#define ADXL345_STATUS_INITIALIZED (1 << 0)

#define ADXL345_UPDATE_INTERVAL 10000 // 10ms

static PT_THREAD(_adxl345_thread(struct adxl345 *self)){
	struct pt *thr = &self->thread; 
	struct pt *bthr = &self->bthread; 
	PT_BEGIN(thr); 
	
	self->status = 0; 
	
	// initialize the device
	self->buffer[0] = 0x31; self->buffer[1] = ADXL345_RANGE | (ADXL345_FULLRANGE<<3);
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2));
	self->buffer[0] = 0x2D; self->buffer[1] = 0x00; // disable
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2));
	self->buffer[0] = 0x2D; self->buffer[1] = 0x16; // standby
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2));
	self->buffer[0] = 0x2D; self->buffer[1] = 0x08; // enable
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2));
	self->buffer[0] = 0x2E; self->buffer[1] = 0x80; // data_ready on int2
	PT_SPAWN(thr, bthr, i2c_write_thread(self->i2c, bthr, self->addr, self->buffer, 2));
	
	self->status |= ADXL345_STATUS_INITIALIZED; 
	
	while(1) {
		PT_WAIT_UNTIL(thr, timestamp_expired(self->time)); 
		
		self->buffer[0] = 0x32; // read register
		PT_SPAWN(thr, bthr, i2c_write_read_thread(self->i2c, bthr, self->addr, self->buffer, 1, 6)); 
		
		self->raw_ax = (uint16_t)self->buffer[0] | ((uint16_t)self->buffer[1] << 8);
		self->raw_ay = (uint16_t)self->buffer[2] | ((uint16_t)self->buffer[3] << 8);
		self->raw_az = (uint16_t)self->buffer[4] | ((uint16_t)self->buffer[5] << 8);
	
		self->time = timestamp_from_now_us(ADXL345_UPDATE_INTERVAL); 
	}
	
	PT_END(thr); 
}

void adxl345_init(struct adxl345 *self, i2c_dev_t i2c, uint8_t addr) {
	self->i2c = i2c;
	self->addr = (addr)?addr:ADXL345_ADDR; 
	PT_INIT(&self->thread); 
	PT_INIT(&self->bthread); 
}

void adxl345_update(struct adxl345 *self){
	_adxl345_thread(self); 
}

/*
static uint8_t _adxl345_read_register(struct adxl345 *self, uint8_t reg) {
	uint8_t value; 
	i2c_start_write(self->i2c, self->addr, &reg, 1);
	i2c_start_read(self->i2c, self->addr, &value, 1);
	i2c_stop(self->i2c);
	return value;
}

static int8_t adxl345_wait_for_data_ready(struct adxl345 *self) {
	//wait until data is ready
	timestamp_t timeout = timestamp_from_now_us(500000); 
	while(!(_adxl345_read_register(self, 0x30) & 0x80)){
		if(timestamp_expired(timeout)) return -1;
	}
	return 0; 
}
*/
int8_t adxl345_read_raw(struct adxl345 *self, int16_t *axraw, int16_t *ayraw, int16_t *azraw){
	*axraw = self->raw_ax; 
	*ayraw = self->raw_ay; 
	*azraw = self->raw_az; 
	return 0; 
}

int8_t adxl345_read_adjusted(struct adxl345 *self, float *ax, float *ay, float *az) {
	//axisg = mx + b
	//m is the scaling factor (g/counts), x is the sensor output (counts), and b is the count offset.
	#if ADXL345_CALIBRATED == 1
	*ax = (self->raw_ax/(float)ADXL345_CALRANGEVALX) + (float)ADXL345_CALOFFSETX;
	*zy = (self->raw_ay/(float)ADXL345_CALRANGEVALY) + (float)ADXL345_CALOFFSETY;
	*az = (self->raw_az/(float)ADXL345_CALRANGEVALZ) + (float)ADXL345_CALOFFSETZ;
	#else
	*ax = (self->raw_ax/(float)ADXL345_RANGEVAL);
	*ay = (self->raw_ay/(float)ADXL345_RANGEVAL);
	*az = (self->raw_az/(float)ADXL345_RANGEVAL);
	#endif

	return 0; 
}

