/*
MMA7455 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef MMA7455_H_
#define MMA7455_H_

//definitions
#define MMA7455_ADDR (0x1D<<1) //device address
#define MMA7455_MODE8BIT 8 //sensitivity to 8bit
#define MMA7455_MODE10BIT 10 //sensitivity to 10bit
#define MMA7455_RANGE2G 0x05 //sensitivity to 2g and measurement mode
#define MMA7455_RANGE4G 0x09 //sensitivity to 4g and measurement mode
#define MMA7455_RANGE8G 0x01 //sensitivity to 8g and measurement mode

//i2c settings
#define MMA7455_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define MMA7455_I2CINIT 1 //init i2c

//settings
#define MMA7455_RANGE MMA7455_RANGE2G //select from sensitivity above
#define MMA7455_LOWPASSENABLED 1 //1 to enable the low pass filter, 0 to disable
#define MMA7455_GETATTITUDE 0 //enable this to get attitude pitch and roll

//automatic definitions
//gravity is 1g, pressure on axis is (counts @ 1g - counts @ -1g) / sensitivity g
//let's suppose for a 2g range a sensitivity of 8-bit (256 counts max, from -128 @ -2g, to 128 @ 2g),
//the value @ 1g should be 128/2, and -128/2 @ -1g, so (64 - -64) / 2 = 64
#if MMA7455_RANGE == MMA7455_RANGE2G
#define MMA7455_MODE MMA7455_MODE8BIT
#define MMA7455_RANGEVAL 64
#elif MMA7455_RANGE == MMA7455_RANGE4G
#define MMA7455_MODE MMA7455_MODE8BIT
#define MMA7455_RANGEVAL 32
#elif MMA7455_RANGE == MMA7455_RANGE8G
#define MMA7455_MODE MMA7455_MODE10BIT
#define MMA7455_RANGEVAL 64
#endif

#define MMA7455_CALIBRATED 1 //enable this if this accel is calibrated
//to calibrate the sensor collect values placing the accellerometer on every position of an ideal cube,
//for example on axis-x you can read -62 @ -1g and +68 @ 1g counts, the scale factor will be 130 / 2,
//offset is = 1g - counts/scale factor, with a value of -62 @1g, -62 / (130/2) = 0.95
//offset should be 1-0.95 = 0.05
#if MMA7455_CALIBRATED == 1
#define MMA7455_CALRANGEVALX 62.5
#define MMA7455_CALRANGEVALY 64.5
#define MMA7455_CALRANGEVALZ 62.5
#define MMA7455_CALOFFSETX 0.12
#define MMA7455_CALOFFSETY 0.27
#define MMA7455_CALOFFSETZ -0.04
#endif

//functions declarations
extern void mma7455_init(void);
extern void mma7455_getdata(double *ax, double *ay, double *az);
#if MMA7455_GETATTITUDE == 1
extern void mma7455_getpitchroll(double ax, double ay, double az, double *pitch, double *roll);
#endif

#endif
