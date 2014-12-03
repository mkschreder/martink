/*
ADXL345 lib 0x02

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef ADXL345_H_
#define ADXL345_H_
//note: we use only 10 bit resolution

//definitions
#define ADXL345_ADDR (0x53<<1) //device address
#define ADXL345_RANGE2G 0x00 //sensitivity to 2g and measurement mode
#define ADXL345_RANGE4G 0x01 //sensitivity to 4g and measurement mode
#define ADXL345_RANGE8G 0x02 //sensitivity to 8g and measurement mode
#define ADXL345_RANGE16G 0x03 //sensitivity to 16g and measurement mode
#define ADXL345_FULLRANGE 0 //1 to enable 0 to disable

//i2c settings
#define ADXL345_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define ADXL345_I2CINIT 1 //init i2c

//settings
#define ADXL345_RANGE ADXL345_RANGE2G //select from sensitivity above
#define ADXL345_LOWPASSENABLED 1 //1 to enable the low pass filter, 0 to disable
#define ADXL345_GETATTITUDE 1 //enable this to get attitude pitch and roll

//automatic definitions
//gravity is 1g, pressure on axis is (counts @ 1g - counts @ -1g) / sensitivity g
//let's suppose for a 2g range a sensitivity of 8-bit (256 counts max, from -128 @ -2g, to 128 @ 2g),
//the value @ 1g should be 128/2, and -128/2 @ -1g, so (64 - -64) / 2 = 64
#if ADXL345_RANGE == ADXL345_RANGE2G
#define ADXL345_RANGEVAL 256
#elif ADXL345_RANGE == ADXL345_RANGE4G
#if ADXL345_FULLRANGE == 0
#define ADXL345_RANGEVAL 128
#else
#define ADXL345_RANGEVAL 256
#endif
#elif ADXL345_RANGE == ADXL345_RANGE8G
#if ADXL345_FULLRANGE == 0
#define ADXL345_RANGEVAL 64
#else
#define ADXL345_RANGEVAL 256
#endif
#elif ADXL345_RANGE == ADXL345_RANGE16G
#if ADXL345_FULLRANGE == 0
#define ADXL345_RANGEVAL 32
#else
#define ADXL345_RANGEVAL 256
#endif
#endif

#define ADXL345_CALIBRATED 0 //enable this if this accel is calibrated
//to calibrate the sensor collect values placing the accellerometer on every position of an ideal cube,
//for example on axis-x you can read -62 @ -1g and +68 @ 1g counts, the scale factor will be 130 / 2,
//offset is = 1g - counts/scale factor, with a value of -62 @1g, -62 / (130/2) = 0.95
//offset should be 1-0.95 = 0.05
#if ADXL345_CALIBRATED == 1
#define ADXL345_CALRANGEVALX 0
#define ADXL345_CALRANGEVALY 0
#define ADXL345_CALRANGEVALZ 0
#define ADXL345_CALOFFSETX 0
#define ADXL345_CALOFFSETY 0
#define ADXL345_CALOFFSETZ 0
#endif

//functions declarations
extern void adxl345_init(void);
extern void adxl345_calibrateoffset(void);
extern void adxl345_getdata(double *ax, double *ay, double *az);
#if ADXL345_GETATTITUDE == 1
extern void adxl345_getpitchroll(double ax, double ay, double az, double *pitch, double *roll);
#endif

#endif
