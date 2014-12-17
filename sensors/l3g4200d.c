/*
l3g4200d lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdlib.h>

#include <arch/soc.h>

#include "l3g4200d.h"

//offset variables
volatile double l3g4200d_offsetx = 0.0f;
volatile double l3g4200d_offsety = 0.0f;
volatile double l3g4200d_offsetz = 0.0f;

//reference temperature
int8_t l3g4200d_temperatureref = 0;
#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
double l3g4200d_gtemp = 0; //temperature used for compensation
#endif

/*
 * set reference temperature
 */
void l3g4200d_settemperatureref(void) {
	uint8_t rawtemp; 
	/*
	i2c_start(L3G4200D_ADDR | I2C_WRITE);
	i2c_write(L3G4200D_OUT_TEMP);
	i2c_rep_start(L3G4200D_ADDR | I2C_READ);
	rawtemp = i2c_readNak();
	i2c_stop();
*/
	l3g4200d_temperatureref = (int8_t) rawtemp;
	#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
	l3g4200d_gtemp = (double)rawtemp;
	#endif
}

/*
 * get temperature variation
 */
int8_t l3g4200d_gettemperaturediff(void) {
	uint8_t rawtemp; 
	/*
	i2c_start(L3G4200D_ADDR | I2C_WRITE);
	i2c_write(L3G4200D_OUT_TEMP);
	i2c_rep_start(L3G4200D_ADDR | I2C_READ);
	rawtemp = i2c_readNak();
	i2c_stop();
*/
	return l3g4200d_temperatureref - (int8_t)rawtemp;
}

/*
 * set offset variables
 */
void l3g4200d_setoffset(double offsetx, double offsety, double offsetz) {
	l3g4200d_offsetx = offsetx;
	l3g4200d_offsety = offsety;
	l3g4200d_offsetz = offsetz;
}

/*
 * get raw data
 */
void l3g4200d_getrawdata(int16_t *gxraw, int16_t *gyraw, int16_t *gzraw) {
	uint8_t i = 0;
	uint8_t buff[6];
/*
	i2c_start(L3G4200D_ADDR | I2C_WRITE);
	i2c_write(L3G4200D_OUT_X_L | (1 << 7));
	i2c_rep_start(L3G4200D_ADDR | I2C_READ);
	for(i=0; i<6; i++) {
		if(i==6-1)
			buff[i] = i2c_readNak();
		else
			buff[i] = i2c_readAck();
	}
	i2c_stop();
*/
	*gxraw = ((buff[1] << 8) | buff[0]);
	*gyraw = ((buff[3] << 8) | buff[2]);
	*gzraw = ((buff[5] << 8) | buff[4]);
}


/*
 * get converted data deg/sec
 */
void l3g4200d_getdata(double* gx, double* gy, double* gz) {
	int16_t gxraw = 0;
	int16_t gyraw = 0;
	int16_t gzraw = 0;
	l3g4200d_getrawdata(&gxraw, &gyraw, &gzraw);

	#if L3G4200D_CALIBRATED == 1 && L3G4200D_CALIBRATEDDOTEMPCOMP == 1
	l3g4200d_gtemp = l3g4200d_gtemp*0.95 + 0.05*l3g4200d_gettemperaturediff(); //filtered temperature compansation
	#endif

	#if L3G4200D_CALIBRATED == 1
		#if L3G4200D_CALIBRATEDDOTEMPCOMP == 1
	*gx = (gxraw - (double)((L3G4200D_TEMPCOMPX*l3g4200d_gtemp) + (double)l3g4200d_offsetx)) * (double)L3G4200D_GAINX;
	*gy = (gyraw - (double)((L3G4200D_TEMPCOMPY*l3g4200d_gtemp) + (double)l3g4200d_offsety)) * (double)L3G4200D_GAINY;
	*gz = (gzraw - (double)((L3G4200D_TEMPCOMPZ*l3g4200d_gtemp) + (double)l3g4200d_offsetz)) * (double)L3G4200D_GAINZ;
		#else
	*gx = (gxraw-(double)l3g4200d_offsetx) * (double)L3G4200D_GAINX;
	*gy = (gyraw-(double)l3g4200d_offsety) * (double)L3G4200D_GAINY;
	*gz = (gzraw-(double)l3g4200d_offsetz) * (double)L3G4200D_GAINZ;
		#endif
	#else
	*gx = (gxraw-(double)l3g4200d_offsetx) * (double)L3G4200D_GAIN;
	*gy = (gyraw-(double)l3g4200d_offsety) * (double)L3G4200D_GAIN;
	*gz = (gzraw-(double)l3g4200d_offsetz) * (double)L3G4200D_GAIN;
	#endif
}

/*
 * init L3G4200D_
 */
void l3g4200d_init(void) {
	//enable chip
	/*
	i2c_start(L3G4200D_ADDR | I2C_WRITE);
	i2c_write(L3G4200D_CTRL_REG1);
	i2c_write(0x0F); //0x0F = 0b00001111, normal power mode, all axes enabled
	i2c_stop();
	//set range
	i2c_start(L3G4200D_ADDR | I2C_WRITE);
	i2c_write(L3G4200D_CTRL_REG4);
	i2c_write(L3G4200D_RANGE<<4);
	i2c_stop();
*/
	#if L3G4200D_CALIBRATED == 1
	//init offset
	l3g4200d_offsetx = L3G4200D_OFFSETX;
	l3g4200d_offsety = L3G4200D_OFFSETY;
	l3g4200d_offsetz = L3G4200D_OFFSETZ;
	#endif

	l3g4200d_settemperatureref();
}
