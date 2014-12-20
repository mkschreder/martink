/**
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

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder

	Special thanks to:
	* Davide Gironi, original implementation
*/

#include <stdlib.h>

#include <arch/soc.h>

#include "adxl345.h"


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

#if ADXL345_LOWPASSENABLED == 1
static float axold = 0;
static float ayold = 0;
static float azold = 0;
static uint8_t firstread = 1;
#endif

/*
 * initialize the accellerometer
 */
void adxl345_init(struct adxl345 *self, struct i2c_interface *i2c, uint8_t addr) {
	self->i2c = i2c;
	self->addr = (addr)?addr:ADXL345_ADDR; 
	uint8_t data[2];
	data[0] = 0x31; data[1] = ADXL345_RANGE | (ADXL345_FULLRANGE<<3);
	i2c->start_write(i2c, self->addr, data, 2);
	data[0] = 0x2D; data[1] = 0x00; // disable
	i2c->start_write(i2c, self->addr, data, 2);
	data[0] = 0x2D; data[1] = 0x16; // standby
	i2c->start_write(i2c, self->addr, data, 2);
	data[0] = 0x2D; data[1] = 0x08; // enable
	i2c->start_write(i2c, self->addr, data, 2);
	data[0] = 0x2E; data[1] = 0x80; // data_ready on int2
	i2c->start_write(i2c, self->addr, data, 2);
	i2c->stop(i2c); 
}

/*
 * write the calibration offset
 */
void adxl345_write_offset(struct adxl345 *self, int8_t offsetx, int8_t offsety, int8_t offsetz) {
	uint8_t data[2];
	data[0] = 0x1E; data[1] = offsetx; 
	self->i2c->start_write(self->i2c, self->addr, data, 2);
	data[0] = 0x1F; data[1] = offsety; 
	self->i2c->start_write(self->i2c, self->addr, data, 2);
	data[0] = 0x20; data[1] = offsetz;
	self->i2c->start_write(self->i2c, self->addr, data, 2);
	self->i2c->stop(self->i2c); 
}

static uint8_t _adxl345_read_register(struct adxl345 *self, uint8_t reg) {
	uint8_t value; 
	self->i2c->start_write(self->i2c, self->addr, &reg, 1);
	self->i2c->start_read(self->i2c, self->addr, &value, 1);
	self->i2c->stop(self->i2c);
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

int8_t adxl345_read_raw(struct adxl345 *self, int16_t *axraw, int16_t *ayraw, int16_t *azraw){
	if(adxl345_wait_for_data_ready(self) == -1) return -1;

	//read axis data
	*axraw = _adxl345_read_register(self, 0x32);
	*axraw += _adxl345_read_register(self, 0x33) << 8;
	*axraw = _adxl345_read_register(self, 0x34);
	*axraw += _adxl345_read_register(self, 0x35) << 8;
	*axraw = _adxl345_read_register(self, 0x36);
	*axraw += _adxl345_read_register(self, 0x37) << 8;

	return 0; 
}

int8_t adxl345_read_adjusted(struct adxl345 *self, float *ax, float *ay, float *az) {
	int16_t axraw = 0;
	int16_t ayraw = 0;
	int16_t azraw = 0;

	if(adxl345_read_raw(self, &axraw, &ayraw, &azraw) == -1)
		return -1;
	
	//axisg = mx + b
	//m is the scaling factor (g/counts), x is the sensor output (counts), and b is the count offset.
	#if ADXL345_CALIBRATED == 1
	*ax = (axraw/(float)ADXL345_CALRANGEVALX) + (float)ADXL345_CALOFFSETX;
	*zy = (ayraw/(float)ADXL345_CALRANGEVALY) + (float)ADXL345_CALOFFSETY;
	*az = (azraw/(float)ADXL345_CALRANGEVALZ) + (float)ADXL345_CALOFFSETZ;
	#else
	*ax = (axraw/(float)ADXL345_RANGEVAL);
	*ay = (ayraw/(float)ADXL345_RANGEVAL);
	*az = (azraw/(float)ADXL345_RANGEVAL);
	#endif

	//this is a simple low pass filter
	#if ADXL345_LOWPASSENABLED == 1
	if(!firstread)
		*ax = (0.75)*(axold) + (0.25)*(*ax);
	axold = *ax;
	if(!firstread)
		*ay = (0.75)*(ayold) + (0.25)*(*ay);
	ayold = *ay;
	if(!firstread)
		*az = (0.75)*(azold) + (0.25)*(*az);
	azold = *az;
	firstread = 0;
	#endif

	return 0; 
}

