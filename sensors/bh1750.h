/*
bh1750 lib 0x02

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef BH1750_H_
#define BH1750_H_


#define BH1750_ADDR (0x23<<1) //device address

//i2c settings
#define BH1750_I2CFLEURYPATH "i2cmaster.h" //define the path to i2c fleury lib
#define BH1750_I2CINIT 1 //init i2c

//resolution modes
#define BH1750_MODEH 0x10 //continuously h-resolution mode, 1lx resolution, 120ms
#define BH1750_MODEH2 0x11 //continuously h-resolution mode, 0.5lx resolution, 120ms
#define BH1750_MODEL 0x13 //continuously l-resolution mode, 4x resolution, 16ms
//define active resolution mode
#define BH1750_MODE BH1750_MODEH

//functions
extern void bh1750_init(void);
extern uint16_t bh1750_getlux(void);


#endif
