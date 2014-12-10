/*
ADXL345 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdlib.h>

#include <arch/soc.h>

#include "adxl345.h"

#if ADXL345_GETATTITUDE == 1
#include <math.h>
#include <string.h>
#endif


#if ADXL345_LOWPASSENABLED == 1
static double axold = 0;
static double ayold = 0;
static double azold = 0;
static uint8_t firstread = 1;
#endif


#if ADXL345_GETATTITUDE == 1
/*
 * estimate pitch and row using euleros angles
 */
void adxl345_getpitchroll(double ax, double ay, double az, double *pitch, double *roll) {
	double magnitude = sqrt((ax * ax) + (ay * ay) + (az * az));
	ax = ax / magnitude;
	ay = ay / magnitude;
	az = az / magnitude;
	*pitch = -atan2(ax, sqrt(pow(ay,2) + pow(az, 2)));
	*roll = atan2(ay, az);
}
#endif

/*
 * initialize the accellerometer
 */
void adxl345_init(void) {
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	uint8_t range = ADXL345_RANGE | (ADXL345_FULLRANGE<<3);
	i2c_write(0x31);
	i2c_write(range);
	//power register
    i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
    i2c_write(0x2D);
    i2c_write(0x0); //disable
    i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
    i2c_write(0x2D);
    i2c_write(0x16); //stand by
    i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
    i2c_write(0x2D);
    i2c_write(0x08); //enable
    //interrupt
    i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x2E);
	i2c_write(0x80); //data_ready on int2

	_delay_ms(20);
	i2c_stop();
}

/*
 * write the calibration offset
 */
void adxl345_writeoffset(int8_t offsetx, int8_t offsety, int8_t offsetz) {
	//x offset
	i2c_start_wait(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x1E);
	i2c_write(offsetx);
	//y offset
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x1F);
	i2c_write(offsety);
	//z offset
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x20);
	i2c_write(offsetz);

	i2c_stop();
}

/*
 * wait for xyz data to be ready
 */
void adxl345_waitfordataready(void) {
	//wait until data is ready
	unsigned char status = 0;
	i2c_start_wait(ADXL345_ADDR | I2C_WRITE);
	do {
		i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
		i2c_write(0x30); //status register
		i2c_rep_start(ADXL345_ADDR | I2C_READ);
		status = i2c_readNak();
		status &= 0b10000000;
	} while (!status);
}

/*
 * get xyz accellerometer values
 */
void adxl345_getdata(double *ax, double *ay, double *az) {
	int16_t axraw = 0;
	int16_t ayraw = 0;
	int16_t azraw = 0;

	adxl345_waitfordataready();

	//read axis data
	int16_t temp;
	//X
	i2c_start_wait(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x32);
	i2c_rep_start(ADXL345_ADDR | I2C_READ);
	axraw = i2c_readNak();
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x32+1);
	i2c_rep_start(ADXL345_ADDR | I2C_READ);
	temp = i2c_readNak();
	axraw += (temp<<8);
	//Y
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x34);
	i2c_rep_start(ADXL345_ADDR | I2C_READ);
	ayraw = i2c_readNak();
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x34+1);
	i2c_rep_start(ADXL345_ADDR | I2C_READ);
	temp = i2c_readNak();
	ayraw += (temp<<8);
	//Z
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x36);
	i2c_rep_start(ADXL345_ADDR | I2C_READ);
	azraw = i2c_readNak();
	i2c_rep_start(ADXL345_ADDR | I2C_WRITE);
	i2c_write(0x36+1);
	i2c_rep_start(ADXL345_ADDR | I2C_READ);
	temp = i2c_readNak();
	azraw += (temp<<8);

	i2c_stop();

	//axisg = mx + b
	//m is the scaling factor (g/counts), x is the sensor output (counts), and b is the count offset.
	#if ADXL345_CALIBRATED == 1
	*ax = (axraw/(double)ADXL345_CALRANGEVALX) + (double)ADXL345_CALOFFSETX;
	*zy = (ayraw/(double)ADXL345_CALRANGEVALY) + (double)ADXL345_CALOFFSETY;
	*az = (azraw/(double)ADXL345_CALRANGEVALZ) + (double)ADXL345_CALOFFSETZ;
	#else
	*ax = (axraw/(double)ADXL345_RANGEVAL);
	*ay = (ayraw/(double)ADXL345_RANGEVAL);
	*az = (azraw/(double)ADXL345_RANGEVAL);
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
}

