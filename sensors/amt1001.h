/*
amt1001 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef AMT1001_H_
#define AMT1001_H_



//functions
extern int16_t amt1001_gethumidity(double voltage);
extern int16_t amt1001_gettemperature(uint16_t adcvalue);

#endif
