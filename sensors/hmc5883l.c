/*
hmc5883l lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdlib.h>

#include <arch/soc.h>
#include "hmc5883l.h"


//registers
#define HMC5883L_CONFREGA 0x00
#define HMC5883L_CONFREGB 0x01
#define HMC5883L_MODEREG 0x02
#define HMC5883L_DATAREGBEGIN 0x03

//setup measurement mode
#define HMC5883L_MEASURECONTINOUS 0x00
#define HMC5883L_MEASURESINGLESHOT 0x01
#define HMC5883L_MEASUREIDLE 0x03
#define HMC5883L_MEASUREMODE HMC5883L_MEASURECONTINOUS

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

#define HMC5883L_CALIBRATED 0 //enable this if this magn is calibrated

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
 * init the hmc5883l
 */
void hmc5883l_init(struct hmc5883l *self, struct i2c_interface *i2c, uint8_t addr, uint8_t scale) {
	self->i2c = i2c;
	self->addr = addr;
	
	delay_us(50000L); 
	
	//set scale
	self->scale = 0;

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
  
  uint8_t buf[] = {
		HMC5883L_CONFREGA, 
		0x70, //Configuration Register A  -- 0 11 100 00  num samples: 8 ; output rate: 15Hz ; normal measurement mode
		HMC5883L_CONFREGB, 
		0x20, //Configuration Register B  -- 001 00000    configuration gain 1.3Ga
		HMC5883L_MODEREG,
		0x00 //Mode register             -- 000000 00    continuous Conversion Mode
	};
	i2c->start_write(i2c, addr, buf, sizeof(buf));
	i2c->stop(i2c);
	
	/*twi0_start_transaction(TWI_OP_LIST(
		TWI_OP(HMC5883L_ADDR | I2C_WRITE, buf, 6)
	)); 
	twi0_wait(); */
  // leave test mode
  /*i2c_start_wait(HMC5883L_ADDR | I2C_WRITE); 
  i2c_write(HMC5883L_CONFREGA); 
  i2c_write(0x70); //Configuration Register A  -- 0 11 100 00  num samples: 8 ; output rate: 15Hz ; normal measurement mode
  i2c_write(HMC5883L_CONFREGB); 
  i2c_write(0x20); //Configuration Register B  -- 001 00000    configuration gain 1.3Ga
  i2c_write(HMC5883L_MODEREG); 
  i2c_write(0x00); //Mode register             -- 000000 00    continuous Conversion Mode
  i2c_stop(); */
  
  delay_us(100000L);
  
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
void hmc5883l_read_raw(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw) {
	uint8_t buff[6] = {HMC5883L_DATAREGBEGIN};
	self->i2c->start_write(self->i2c, self->addr, buff, 1);
	self->i2c->start_read(self->i2c, self->addr, buff, 6);
	self->i2c->stop(self->i2c);
	
	*mxraw = (int16_t)((buff[0] << 8) | buff[1]);
	*mzraw = (int16_t)((buff[2] << 8) | buff[3]);
	*myraw = (int16_t)((buff[4] << 8) | buff[5]);
}

/*
 * get scaled data
 */
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
	/*
	*mx = mxraw * self->scale;
	*my = myraw * self->scale;
	*mz = mzraw * self->scale;*/
	*mx = mxraw;
	*my = myraw;
	*mz = mzraw; 
	#endif
}
