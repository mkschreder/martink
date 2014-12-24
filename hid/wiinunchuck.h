/*
wiinunchuck 0x02

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#ifndef WIINUNCHUCK_H_
#define WIINUNCHUCK_H_


//enable a more precise function for centering joypad
//0 to select o static offset center
//1 to enable a math function to center the joypad
#define WIINUNCHUCK_JOYCENTERB 1

//get non calibrated values, use this function to calibrate the device
#define WIINUNCHUCK_GETNONCALIBRATED 0

//pulse button enabled
#define WIINUNCHUCK_PULSEBUTTON 0

//angle filter enabled
#define WIINUNCHUCK_ANGLEFILTER 1
#define WIINUNCHUCK_ANGLEAVARAGECOEF 21 //number of samples to avarage

struct wiinunchuck {
	i2c_dev_t i2c;
	
	uint8_t joyX;
	uint8_t joyY;
	#if WIINUNCHUCK_PULSEBUTTON == 1
	uint8_t lastbuttonZ;
	uint8_t lastbuttonC;
	#endif
	uint8_t buttonZ;
	uint8_t buttonC;
	int angleX;
	int angleY;
	int angleZ;
	#if WIINUNCHUCK_ANGLEFILTER == 1
	int avarageangleX[WIINUNCHUCK_ANGLEAVARAGECOEF];
	int avarageangleY[WIINUNCHUCK_ANGLEAVARAGECOEF];
	int avarageangleZ[WIINUNCHUCK_ANGLEAVARAGECOEF];
	#endif
};

//functions
extern void wiinunchuck_calibratejoy(struct wiinunchuck *self);
extern void wiinunchuck_init(struct wiinunchuck *self, i2c_dev_t i2c);
extern int wiinunchuck_getjoyX(struct wiinunchuck *self);
extern int wiinunchuck_getjoyY(struct wiinunchuck *self);
extern uint8_t wiinunchuck_getbuttonZ(struct wiinunchuck *self);
extern uint8_t wiinunchuck_getbuttonC(struct wiinunchuck *self);
extern int wiinunchuck_getangleX(struct wiinunchuck *self);
extern int wiinunchuck_getangleY(struct wiinunchuck *self);
extern int wiinunchuck_getangleZ(struct wiinunchuck *self);
extern void wiinunchuck_update(struct wiinunchuck *self);
extern void wiinunchuck_getpitchroll(struct wiinunchuck *self, double ax, double ay, double az, double *pitch, double *roll);
#endif

