/*
l3g4200d lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  - Arduino library for the Pololu L3G4200D carrier board
    https://github.com/pololu/L3G4200D
*/


#ifndef L3G4200D_H_
#define L3G4200D_H_

#ifdef __cplusplus
extern "C" {
#endif

//definitions
#define L3G4200D_ADDR (0x69<<1) //device address

//i2c settings
#define L3G4200D_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define L3G4200D_I2CINIT 1 //init i2c

//set range
#define L3G4200D_RANGE250 0
#define L3G4200D_RANGE500 1
#define L3G4200D_RANGE2000 2
#define L3G4200D_RANGE L3G4200D_RANGE2000

//define DPS conversion
#if L3G4200D_RANGE == L3G4200D_RANGE250
#define L3G4200D_GAIN 0.00875
#elif L3G4200D_RANGE == L3G4200D_RANGE500
#define L3G4200D_GAIN 0.0175
#elif L3G4200D_RANGE == L3G4200D_RANGE2000
#define L3G4200D_GAIN 0.07
#endif

#define L3G4200D_CALIBRATED 0 //set to 1 if calibrated
#define L3G4200D_CALIBRATEDDOTEMPCOMP 0 //set to 1 to enable temperature compensation

#if L3G4200D_CALIBRATED == 1
//calibration values
#define L3G4200D_OFFSETX -17.04
#define L3G4200D_OFFSETY -6.56
#define L3G4200D_OFFSETZ -20.50
#define L3G4200D_GAINX 0.068
#define L3G4200D_GAINY 0.073
#define L3G4200D_GAINZ 0.069
#if L3G4200D_CALIBRATEDDOTEMPCOMP == 1
#define L3G4200D_TEMPCOMPX -0.002725
#define L3G4200D_TEMPCOMPY 0.000421
#define L3G4200D_TEMPCOMPZ 0.005629
#endif
#endif

//registers
#define L3G4200D_WHO_AM_I      0x0F

#define L3G4200D_CTRL_REG1     0x20
#define L3G4200D_CTRL_REG2     0x21
#define L3G4200D_CTRL_REG3     0x22
#define L3G4200D_CTRL_REG4     0x23
#define L3G4200D_CTRL_REG5     0x24
#define L3G4200D_REFERENCE     0x25
#define L3G4200D_OUT_TEMP      0x26
#define L3G4200D_STATUS_REG    0x27

#define L3G4200D_OUT_X_L       0x28
#define L3G4200D_OUT_X_H       0x29
#define L3G4200D_OUT_Y_L       0x2A
#define L3G4200D_OUT_Y_H       0x2B
#define L3G4200D_OUT_Z_L       0x2C
#define L3G4200D_OUT_Z_H       0x2D

#define L3G4200D_FIFO_CTRL_REG 0x2E
#define L3G4200D_FIFO_SRC_REG  0x2F

#define L3G4200D_INT1_CFG      0x30
#define L3G4200D_INT1_SRC      0x31
#define L3G4200D_INT1_THS_XH   0x32
#define L3G4200D_INT1_THS_XL   0x33
#define L3G4200D_INT1_THS_YH   0x34
#define L3G4200D_INT1_THS_YL   0x35
#define L3G4200D_INT1_THS_ZH   0x36
#define L3G4200D_INT1_THS_ZL   0x37
#define L3G4200D_INT1_DURATION 0x38

//functions
extern void l3g4200d_init(void);
extern void l3g4200d_setoffset(double offsetx, double offsety, double offsetz);
extern void l3g4200d_getrawdata(int16_t *gxraw, int16_t *gyraw, int16_t *gzraw);
extern void l3g4200d_getdata(double* gx, double* gy, double* gz);
extern void l3g4200d_settemperatureref(void);
extern int8_t l3g4200d_gettemperaturediff(void);

#ifdef __cplusplus
}
#endif

#endif
