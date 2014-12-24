/*
hmc5883l lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - HMC5883L Triple Axis Magnetometer Arduino Library
    http://bildr.org/2012/02/hmc5883l_arduino/
*/


#ifndef HMC5883L_H_
#define HMC5883L_H_

#ifdef __cplusplus
extern "C" {
#endif

//definitions
#define HMC5883L_ADDR (0x1E<<1) //device address

struct hmc5883l {
	i2c_dev_t i2c;
	uint8_t addr;
	float scale; 
};

//functions
void hmc5883l_init(struct hmc5883l *self, i2c_dev_t i2c, uint8_t addr, uint8_t scale);
void hmc5883l_read_raw(struct hmc5883l *self, int16_t *mxraw, int16_t *myraw, int16_t *mzraw);
void hmc5883l_read_adjusted(struct hmc5883l *self, float *mx, float *my, float *mz);

#ifdef __cplusplus
}
#endif

#endif
