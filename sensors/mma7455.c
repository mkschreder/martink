/*
	MMA7455 accelerometer driver
	
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

#include "mma7455.h"

#define MMA7455_MODE8BIT 8 //sensitivity to 8bit
#define MMA7455_MODE10BIT 10 //sensitivity to 10bit
#define MMA7455_RANGE2G 0x05 //sensitivity to 2g and measurement mode
#define MMA7455_RANGE4G 0x09 //sensitivity to 4g and measurement mode
#define MMA7455_RANGE8G 0x01 //sensitivity to 8g and measurement mode

//settings
#define MMA7455_RANGE MMA7455_RANGE2G //select from sensitivity above
#define MMA7455_LOWPASSENABLED 1 //1 to enable the low pass filter, 0 to disable
#define MMA7455_GETATTITUDE 0 //enable this to get attitude pitch and roll

//automatic definitions
//gravity is 1g, pressure on axis is (counts @ 1g - counts @ -1g) / sensitivity g
//let's suppose for a 2g range a sensitivity of 8-bit (256 counts max, from -128 @ -2g, to 128 @ 2g),
//the value @ 1g should be 128/2, and -128/2 @ -1g, so (64 - -64) / 2 = 64
#if MMA7455_RANGE == MMA7455_RANGE2G
#define MMA7455_MODE MMA7455_MODE8BIT
#define MMA7455_RANGEVAL 64
#elif MMA7455_RANGE == MMA7455_RANGE4G
#define MMA7455_MODE MMA7455_MODE8BIT
#define MMA7455_RANGEVAL 32
#elif MMA7455_RANGE == MMA7455_RANGE8G
#define MMA7455_MODE MMA7455_MODE10BIT
#define MMA7455_RANGEVAL 64
#endif

#define MMA7455_CALIBRATED 1 //enable this if this accel is calibrated
//to calibrate the sensor collect values placing the accellerometer on every position of an ideal cube,
//for example on axis-x you can read -62 @ -1g and +68 @ 1g counts, the scale factor will be 130 / 2,
//offset is = 1g - counts/scale factor, with a value of -62 @1g, -62 / (130/2) = 0.95
//offset should be 1-0.95 = 0.05
#if MMA7455_CALIBRATED == 1
#define MMA7455_CALRANGEVALX 62.5
#define MMA7455_CALRANGEVALY 64.5
#define MMA7455_CALRANGEVALZ 62.5
#define MMA7455_CALOFFSETX 0.12
#define MMA7455_CALOFFSETY 0.27
#define MMA7455_CALOFFSETZ -0.04
#endif

#if MMA7455_GETATTITUDE == 1
#include <math.h>
#include <string.h>
#endif

#if MMA7455_LOWPASSENABLED == 1
static float axold = 0;
static float ayold = 0;
static float azold = 0;
static uint8_t firstread = 1;
#endif

void mma7455_init(struct mma7455 *self, i2c_dev_t i2c, uint8_t addr, uint8_t mode) {
	self->i2c = i2c;
	self->addr = addr;
	self->mode = mode;
	
	uint8_t data[2] = {0x16, MMA7455_RANGE};
	i2c_start_write(i2c, addr, data, 2);
	i2c_stop(i2c);
}

#if MMA7455_GETATTITUDE == 1
/*
 * estimate pitch and row using euleros angles
 */
void mma7455_getpitchroll(struct mma7455 *self, float ax, float ay, float az, float *pitch, float *roll) {
	float magnitude = sqrt((ax * ax) + (ay * ay) + (az * az));
	ax = ax / magnitude;
	ay = ay / magnitude;
	az = az / magnitude;
	*pitch = -atan2(ax, sqrt(pow(ay,2) + pow(az, 2)));
	*roll = atan2(ay, az);
}
#endif

static uint8_t mma7455_read_reg(struct mma7455 *self, uint8_t reg){
	uint8_t data; 
	i2c_start_write(self->i2c, self->addr, &reg, 1);
	i2c_start_read(self->i2c, self->addr, &data, 1);
	i2c_stop(self->i2c);
	return data; 
}

/*
 * wait for xyz data to be ready
 */
static int8_t mma7455_waitfordataready(struct mma7455 *self) {
	//wait until data is ready
	uint32_t timeout = 100000; 
	while(!(mma7455_read_reg(self, 0x09) & 0x01)){
		timeout--;
		if(timeout == 0) return -1; 
		delay_us(100);
	}
	return 0; 
}

/*
 * get xyz accellerometer values
 */
void mma7455_getdata(struct mma7455 *self, float *ax, float *ay, float *az) {

	#if MMA7455_MODE == MMA7455_MODE8BIT
	int8_t axraw = 0;
	int8_t ayraw = 0;
	int8_t azraw = 0;
	#elif MMA7455_MODE == MMA7455_MODE10BIT
	int16_t axraw = 0;
	int16_t ayraw = 0;
	int16_t azraw = 0;
	#endif

	//wait for data
	mma7455_waitfordataready(self);
	switch(self->mode) {
		case MMA7455_MODE8BIT:
			axraw = mma7455_read_reg(self, 0x06);
			ayraw = mma7455_read_reg(self, 0x07);
			azraw = mma7455_read_reg(self, 0x08);
			break;
		case MMA7455_MODE10BIT:
			axraw = mma7455_read_reg(self, 0x00);
			axraw += (mma7455_read_reg(self, 0x01) << 8);
			ayraw = mma7455_read_reg(self, 0x02);
			ayraw += (mma7455_read_reg(self, 0x03) << 8);
			azraw = mma7455_read_reg(self, 0x04);
			azraw += (mma7455_read_reg(self, 0x05) << 8);
			break;
	}
	
	//transform raw data to g data
	//axisg = mx + b
	//m is the scaling factor (g/counts), x is the sensor output (counts), and b is the count offset.
	#if MMA7455_CALIBRATED == 1
	*ax = (axraw/(float)MMA7455_CALRANGEVALX) + (float)MMA7455_CALOFFSETX;
	*ay = (ayraw/(float)MMA7455_CALRANGEVALY) + (float)MMA7455_CALOFFSETY;
	*az = (azraw/(float)MMA7455_CALRANGEVALZ) + (float)MMA7455_CALOFFSETZ;
	#else
	*ax = (axraw/(float)MMA7455_RANGEVAL);
	*ay = (ayraw/(float)MMA7455_RANGEVAL);
	*az = (azraw/(float)MMA7455_RANGEVAL);
	#endif

	//this is a simple low pass filter
	#if MMA7455_LOWPASSENABLED == 1
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
}


