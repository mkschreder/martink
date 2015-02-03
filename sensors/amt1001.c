/*
	This file is part of martink project. 

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Github: https://github.com/mkschreder

	Contributors:
	* Davide Gironi - developing original driver
	* Martin K. Schröder - maintenance since Oct 2014
*/

#include <stdio.h>

#include <arch/soc.h>

#include "amt1001.h"

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


//define min and max valid voltage to measure humidity
#define AMT1001_HUMIDITYVMIN 0.0
#define AMT1001_HUMIDITYVMAX 3.0

//slope factor to calculate humidity
#define AMT1001_HUMIDITYSLOPE 33.33

//define lookup method variables for temperature
#define amt1001_lookupadcfirst 1 //adc first value of lookup table
#define amt1001_lookupadcstep 10 //step between every table point
#define amt1001_lookuptablesize 103 //size of the lookup table
const float PROGMEM amt1001_lookuptable[amt1001_lookuptablesize] = {
		-86.99 , -59.05 , -49.91 , -43.96 , -39.45 , -35.76 , -32.61 , -29.84 , -27.37 , -25.11 , -23.03 , -21.1 , -19.29 , -17.58 , -15.95 , -14.41 , -12.92 , -11.5 , -10.12 , -8.79 , -7.5 , -6.25 , -5.03 , -3.83 , -2.66 , -1.52 , -0.4 , 0.71 , 1.8 , 2.87 , 3.93 , 4.97 , 6.01 , 7.03 , 8.05 , 9.05 , 10.06 , 11.05 , 12.04 , 13.03 , 14.02 , 15 , 15.98 , 16.96 , 17.95 , 18.93 , 19.92 , 20.9 , 21.9 , 22.89 , 23.89 , 24.9 , 25.91 , 26.93 , 27.96 , 29 , 30.05 , 31.11 , 32.19 , 33.27 , 34.37 , 35.49 , 36.63 , 37.78 , 38.95 , 40.14 , 41.36 , 42.6 , 43.87 , 45.17 , 46.5 , 47.86 , 49.26 , 50.7 , 52.18 , 53.71 , 55.29 , 56.92 , 58.62 , 60.38 , 62.22 , 64.14 , 66.15 , 68.26 , 70.49 , 72.84 , 75.34 , 78.01 , 80.87 , 83.95 , 87.3 , 90.96 , 95 , 99.52 , 104.62 , 110.48 , 117.36 , 125.67 , 136.09 , 149.94 , 170.2 , 206.11 , 330.42
};

/*
 * get humidity based on read voltage
 */
int16_t amt1001_gethumidity(double voltage) {
	if(voltage > AMT1001_HUMIDITYVMIN && voltage < AMT1001_HUMIDITYVMAX)
		return (int16_t)(AMT1001_HUMIDITYSLOPE*voltage);
	else
		return -1;
}

/*
 * get temperature based on read voltage
 */
int16_t amt1001_gettemperature(uint16_t adcvalue) {
	float t = 0.0;
	float mint = 0;
	float maxt = 0;

	//return error for invalid adcvalues
	if(adcvalue<amt1001_lookupadcfirst || adcvalue>amt1001_lookupadcfirst+amt1001_lookupadcstep*(amt1001_lookuptablesize-1)) {
		return -1;
	}

	uint8_t i = 0;
	uint16_t a = amt1001_lookupadcfirst;
	for(i=0; i<amt1001_lookuptablesize; i++) {
		if(adcvalue < a)
			break;
		a += amt1001_lookupadcstep;
	}

	maxt = pgm_read_float(&amt1001_lookuptable[i]); //highest interval value
	if(i==0)
		mint = maxt;
	else
		mint = pgm_read_float(&amt1001_lookuptable[i-1]); //smallest interval value

	//do interpolation
	a = a-amt1001_lookupadcstep;
	t = mint + ((maxt-mint)/amt1001_lookupadcstep) * (adcvalue-a);

	return t;

}
