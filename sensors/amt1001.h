/*
amt1001 0x01

copyright (c) Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef AMT1001_H_
#define AMT1001_H_


//define min and max valid voltage to measure humidity
#define AMT1001_HUMIDITYVMIN 0.0
#define AMT1001_HUMIDITYVMAX 3.0

//slope factor to calculate humidity
#define AMT1001_HUMIDITYSLOPE 33.33

//define min and max valid adc to measure temperature
#define AMT1001_TEMPERATUREVMIN 0.0
#define AMT1001_TEMPERATUREVMAX 0.8

//slope factor to calculate temperature
#define AMT1001_TEMPERATURESLOPE 100.0


//functions
extern int16_t amt1001_gethumidity(double voltage);
extern int16_t amt1001_gettemperature(uint16_t adcvalue);

#endif
