/*
MMA7455 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <stdlib.h>

#include <arch/soc.h>

#include "mma7455.h"

#if MMA7455_GETATTITUDE == 1
#include <math.h>
#include <string.h>
#endif

#if MMA7455_LOWPASSENABLED == 1
static double axold = 0;
static double ayold = 0;
static double azold = 0;
static uint8_t firstread = 1;
#endif


/*
 * initialize the accellerometer
 */
void mma7455_init(void) {
	#if MMA7455_I2CINIT == 1
	//init i2c
	i2c_init();
	_delay_us(10);
	#endif

	// write config
    i2c_start_wait(MMA7455_ADDR | I2C_WRITE);
    i2c_write(0x16);
    i2c_write(MMA7455_RANGE);
    i2c_stop();
}

#if MMA7455_GETATTITUDE == 1
/*
 * estimate pitch and row using euleros angles
 */
void mma7455_getpitchroll(double ax, double ay, double az, double *pitch, double *roll) {
	double magnitude = sqrt((ax * ax) + (ay * ay) + (az * az));
	ax = ax / magnitude;
	ay = ay / magnitude;
	az = az / magnitude;
	*pitch = -atan2(ax, sqrt(pow(ay,2) + pow(az, 2)));
	*roll = atan2(ay, az);
}
#endif

/*
 * wait for xyz data to be ready
 */
void mma7455_waitfordataready(void) {
	//wait until data is ready
	unsigned char status = 0;
	i2c_start_wait(MMA7455_ADDR | I2C_WRITE);
	do {
		i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
		i2c_write(0x09); //status register
		i2c_rep_start(MMA7455_ADDR | I2C_READ);
		status = i2c_readNak();
		status &= (1<<0); //read DRDY to see if data is read
	} while (!status);
}

/*
 * get xyz accellerometer values
 */
void mma7455_getdata(double *ax, double *ay, double *az) {

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
	mma7455_waitfordataready();

	//read raw axis data
	#if MMA7455_MODE == MMA7455_MODE8BIT
	//X
	i2c_start_wait(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x06);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	axraw = i2c_readNak();
	//Y
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x07);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	ayraw = i2c_readNak();
	//Z
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x08);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	azraw = i2c_readNak();
	#elif MMA7455_MODE == MMA7455_MODE10BIT
	int16_t temp;
	//X
	i2c_start_wait(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x00);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	axraw = i2c_readNak();
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x00+1);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	temp = i2c_readNak();
	axraw += (temp<<8);
	if (axraw&0x0200) axraw |= 0xfc00;
	//Y
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x02);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	ayraw = i2c_readNak();
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x02+1);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	temp = i2c_readNak();
	ayraw += (temp<<8);
	if (ayraw&0x0200) ayraw |= 0xfc00;
	//Z
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x04);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	azraw = i2c_readNak();
	i2c_rep_start(MMA7455_ADDR | I2C_WRITE);
	i2c_write(0x04+1);
	i2c_rep_start(MMA7455_ADDR | I2C_READ);
	temp = i2c_readNak();
	azraw += (temp<<8);
	if (azraw&0x0200) azraw |= 0xfc00;
	#endif
	i2c_stop();

	//transform raw data to g data
	//axisg = mx + b
	//m is the scaling factor (g/counts), x is the sensor output (counts), and b is the count offset.
	#if MMA7455_CALIBRATED == 1
	*ax = (axraw/(double)MMA7455_CALRANGEVALX) + (double)MMA7455_CALOFFSETX;
	*ay = (ayraw/(double)MMA7455_CALRANGEVALY) + (double)MMA7455_CALOFFSETY;
	*az = (azraw/(double)MMA7455_CALRANGEVALZ) + (double)MMA7455_CALOFFSETZ;
	#else
	*ax = (axraw/(double)MMA7455_RANGEVAL);
	*ay = (ayraw/(double)MMA7455_RANGEVAL);
	*az = (azraw/(double)MMA7455_RANGEVAL);
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


