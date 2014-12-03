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

//i2c settings
#define HMC5883L_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define HMC5883L_I2CINIT 0 //init i2c

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
#define HMC5883L_SCALE088 1 //0.88
#define HMC5883L_SCALE13 2 //1.3
#define HMC5883L_SCALE19 3 //1.9
#define HMC5883L_SCALE25 4 //2.5
#define HMC5883L_SCALE40 5 //4.0
#define HMC5883L_SCALE47 6 //4.7
#define HMC5883L_SCALE56 7 //5.6
#define HMC5883L_SCALE81 8 //8.1
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

//functions
extern void hmc5883l_init(void);
extern void hmc5883l_getrawdata(int16_t *mxraw, int16_t *myraw, int16_t *mzraw);
extern void hmc5883l_getdata(double *mx, double *my, double *mz);

#ifdef __cplusplus
}
#endif

#endif
