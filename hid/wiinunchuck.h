/*
wiinunchuck 0x02

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifndef WIINUNCHUCK_H_
#define WIINUNCHUCK_H_

//definitions
#define WIINUNCHUCK_ADDR (0x52<<1) //device address
#define WIINUNCHUCK_READBYTES 6

//enable a more precise function for centering joypad
//0 to select o static offset center
//1 to enable a math function to center the joypad
#define WIINUNCHUCK_JOYCENTERB 1

//enable the attitude estimation
#define WIINUNCHUCK_GETATTITUDE 1

//i2c settings
#define WIINUNCHUCK_I2CINIT 1 //enable or disable i2c init
#define WIINUNCHUCK_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib

//get non calibrated values, use this function to calibrate the device
#define WIINUNCHUCK_GETNONCALIBRATED 0

//pulse button enabled
#define WIINUNCHUCK_PULSEBUTTON 0

//angle filter enabled
#define WIINUNCHUCK_ANGLEFILTER 1
#define WIINUNCHUCK_ANGLEAVARAGECOEF 21 //number of samples to avarage

//calibration
#define WIINUNCHUCK_ZEROANGLEX 527
#define WIINUNCHUCK_ZEROANGLEY 531
#define WIINUNCHUCK_ZEROANGLEZ 530
#if WIINUNCHUCK_JOYCENTERB == 1
#define WIINUNCHUCK_CENTERJOYX 137 //the center as read
#define WIINUNCHUCK_INMINJOYX 0 //the min as read
#define WIINUNCHUCK_INMAXJOYX 255 //the max as read
#define WIINUNCHUCK_CENTERJOYY 137 //the center as read
#define WIINUNCHUCK_INMINJOYY 0 //the min as read
#define WIINUNCHUCK_INMAXJOYY 255 //the max as read
#else
#define WIINUNCHUCK_DEFAULTZEROJOYX 137 //the center as read
#define WIINUNCHUCK_DEFAULTZEROJOYY 136 //the center as read
#endif

//decodebyte is necessary with certain initializations
#define WIINUNCHUCK_DECODEBYTE 0 //enable or disable the decode byte function
#if WIINUNCHUCK_DECODEBYTE == 1
#define wiinunchuck_decode(x) (x ^ 0x17) + 0x17;
#endif

//functions
extern void wiinunchuck_calibratejoy(void);
extern void wiinunchuck_init(void);
extern int wiinunchuck_getjoyX(void);
extern int wiinunchuck_getjoyY(void);
extern uint8_t wiinunchuck_getbuttonZ(void);
extern uint8_t wiinunchuck_getbuttonC(void);
extern int wiinunchuck_getangleX(void);
extern int wiinunchuck_getangleY(void);
extern int wiinunchuck_getangleZ(void);
extern void wiinunchuck_update(void);
#if WIINUNCHUCK_GETATTITUDE == 1
extern void wiinunchuck_getpitchroll(double ax, double ay, double az, double *pitch, double *roll);
#endif

#endif
