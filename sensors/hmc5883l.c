/*
hmc5883l lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdlib.h>

#include <arch/soc.h>
#include "hmc5883l.h"

static double hmc5883l_scale = 0;

/*
 * init the hmc5883l
 */
void hmc5883l_init(void) {
	time_delay(50000L); 
	
	//set scale
	hmc5883l_scale = 0;
	uint8_t regValue = 0x00;
	#if HMC5883L_SCALE == HMC5883L_SCALE088
		regValue = 0x00;
		hmc5883l_scale = 0.73;
	#elif HMC5883L_SCALE == HMC5883L_SCALE13
		regValue = 0x01;
		hmc5883l_scale = 0.92;
	#elif HMC5883L_SCALE == HMC5883L_SCALE19
		regValue = 0x02;
		hmc5883l_scale = 1.22;
	#elif HMC5883L_SCALE == HMC5883L_SCALE25
		regValue = 0x03;
		hmc5883l_scale = 1.52;
	#elif HMC5883L_SCALE == HMC5883L_SCALE40
		regValue = 0x04;
		hmc5883l_scale = 2.27;
	#elif HMC5883L_SCALE == HMC5883L_SCALE47
		regValue = 0x05;
		hmc5883l_scale = 2.56;
	#elif HMC5883L_SCALE == HMC5883L_SCALE56
		regValue = 0x06;
		hmc5883l_scale = 3.03;
	#elif HMC5883L_SCALE == HMC5883L_SCALE81
		regValue = 0x07;
		hmc5883l_scale = 4.35;
	#endif

	
  time_delay(50000L);  //Wait before start
  
  // leave test mode
  i2c_start_wait(HMC5883L_ADDR | I2C_WRITE); 
  i2c_write(HMC5883L_CONFREGA); 
  i2c_write(0x70); //Configuration Register A  -- 0 11 100 00  num samples: 8 ; output rate: 15Hz ; normal measurement mode
  i2c_write(HMC5883L_CONFREGB); 
  i2c_write(0x20); //Configuration Register B  -- 001 00000    configuration gain 1.3Ga
  i2c_write(HMC5883L_MODEREG); 
  i2c_write(0x00); //Mode register             -- 000000 00    continuous Conversion Mode
  i2c_stop(); 
  time_delay(100000L);
  
	/*//setting is in the top 3 bits of the register.
	regValue = regValue << 5;
    i2c_start_wait(HMC5883L_ADDR | I2C_WRITE);
    i2c_write(HMC5883L_CONFREGB);
    i2c_write(regValue);
    i2c_stop();

	//set measurement mode
	i2c_start_wait(HMC5883L_ADDR | I2C_WRITE);
	i2c_write(HMC5883L_MODEREG);
	i2c_write(HMC5883L_MEASUREMODE);
	i2c_stop();*/
}

/*
 * get raw data
 */
void hmc5883l_getrawdata(int16_t *mxraw, int16_t *myraw, int16_t *mzraw) {
	uint8_t i = 0;
	uint8_t buff[6];

	i2c_start_wait(HMC5883L_ADDR | I2C_WRITE);
	i2c_write(HMC5883L_DATAREGBEGIN);
	i2c_stop();
	i2c_start_wait(HMC5883L_ADDR | I2C_READ);
	for(i=0; i<6; i++) {
		if(i==6-1)
			buff[i] = i2c_readNak();
		else
			buff[i] = i2c_readAck();
	}
	i2c_stop();

	*mxraw = (int16_t)((buff[0] << 8) | buff[1]);
	*mzraw = (int16_t)((buff[2] << 8) | buff[3]);
	*myraw = (int16_t)((buff[4] << 8) | buff[5]);
}

/*
 * get scaled data
 */
void hmc5883l_getdata(double *mx, double *my, double *mz) {
	int16_t mxraw = 0;
	int16_t myraw = 0;
	int16_t mzraw = 0;
	hmc5883l_getrawdata(&mxraw, &myraw, &mzraw);

	#if HMC5883L_CALIBRATED == 1
	float mxt = mxraw - HMC5883L_OFFSETX;
	float myt = myraw - HMC5883L_OFFSETY;
	float mzt = mzraw - HMC5883L_OFFSETZ;
	*mx = HMC5883L_GAINX1 * mxt + HMC5883L_GAINX2 * myt + HMC5883L_GAINX3 * mzt;
	*my = HMC5883L_GAINY1 * mxt + HMC5883L_GAINY2 * myt + HMC5883L_GAINY3 * mzt;
	*mz = HMC5883L_GAINZ1 * mxt + HMC5883L_GAINZ2 * myt + HMC5883L_GAINZ3 * mzt;
	#else
	/*
	*mx = mxraw * hmc5883l_scale;
	*my = myraw * hmc5883l_scale;
	*mz = mzraw * hmc5883l_scale;*/
	*mx = mxraw;
	*my = myraw;
	*mz = mzraw; 
	#endif
}
