/*
ntctemp 0x02

copyright (c) Davide Gironi, 2011

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include "ntctemp.h"

#if NTCTEMP_B == 1 ||  NTCTEMP_SH == 1
#include <math.h>
#endif

#if NTCTEMP_LKP == 1
#include <avr/pgmspace.h>
#endif

//define lookup method variables
#if NTCTEMP_LKP == 1
#define ntctemp_lookupadcfirst 1 //adc first value of lookup table
#define ntctemp_lookupadcstep 10 //step between every table point
#define ntctemp_lookuptablesize 103 //size of the lookup table
const float PROGMEM ntctemp_lookuptable[ntctemp_lookuptablesize] = {
		-86.28, -58.44, -49.35, -43.43, -38.94, -35.27, -32.14, -29.39, -26.93, -24.69, -22.63, -20.71, -18.91, -17.21, -15.6, -14.06, -12.59, -11.17, -9.81, -8.49, -7.21, -5.97, -4.75, -3.57, -2.41, -1.28, -0.16, 0.93, 2.01, 3.07, 4.12, 5.16, 6.19, 7.2, 8.21, 9.21, 10.2, 11.19, 12.17, 13.15, 14.12, 15.1, 16.07, 17.04, 18.02, 18.99, 19.97, 20.94, 21.93, 22.91, 23.9, 24.9, 25.9, 26.91, 27.93, 28.96, 30, 31.05, 32.11, 33.19, 34.28, 35.38, 36.5, 37.64, 38.8, 39.98, 41.19, 42.42, 43.67, 44.95, 46.27, 47.61, 49, 50.42, 51.88, 53.39, 54.95, 56.57, 58.24, 59.99, 61.8, 63.7, 65.68, 67.76, 69.96, 72.28, 74.75, 77.38, 80.2, 83.24, 86.54, 90.15, 94.13, 98.58, 103.6, 109.37, 116.14, 124.31, 134.55, 148.16, 168.04, 203.19, 324.25
};
#endif

/*
 * get temperature using Beta Model Equation
 *
 * "adcresistance" adc resistence read
 * "beta" beta value
 * "adctref" temperature reference for the measuread value
 * "adcrref" resistance reference for the measured value
 */
#if NTCTEMP_B == 1
float ntctemp_getB(long adcresistence, int beta, float adctref, int adcrref) {
	// use the Beta Model Equation
	// temperature (kelvin) = beta / ( beta / tref + ln ( R / rref ) )
	float t;
	t = beta / ( beta / (float)(adctref + 273.15) + log ( adcresistence / (float)adcrref ) );
	t = t - 273.15; // convert Kelvin to Celcius
	//t = (t * 9.0) / 5.0 + 32.0; // convert Celcius to Fahrenheit
	return t;
}
#endif

/*
 * get temperature using the Steinhart-Hart Thermistor Equation
 *
 * "adcresistance" adc resistence read
 * "A", "B", "C" equation parameters
 */
#if NTCTEMP_SH == 1
float ntctemp_getSH(long adcresistence, float A, float B, float C) {
	// use the Steinhart-Hart Thermistor Equation
	// temperature (Kelvin) = 1 / (A + B*ln(R) + C*(ln(R)^3))
	float t;
	t = log( adcresistence );
	t = 1 / (A + (B * t) + (C * t * t * t));
	t = t - 273.15; // convert Kelvin to Celcius
	//t = (t * 9.0) / 5.0 + 32.0; // convert Celcius to Fahrenheit
	return t;
}
#endif

/*
 * get temperature using a lookup table
 */
#if NTCTEMP_LKP == 1
float ntctemp_getLookup(uint16_t adcvalue) {
	float t = 0.0;
	float mint = 0;
	float maxt = 0;

	//return error for invalid adcvalues
	if(adcvalue<ntctemp_lookupadcfirst || adcvalue>ntctemp_lookupadcfirst+ntctemp_lookupadcstep*(ntctemp_lookuptablesize-1)) {
			return NTCTEMP_LOOKUPRETERROR;
	}

	uint8_t i = 0;
	uint16_t a = ntctemp_lookupadcfirst;
	for(i=0; i<ntctemp_lookuptablesize; i++) {
		if(adcvalue < a)
			break;
		a += ntctemp_lookupadcstep;
	}

	maxt = pgm_read_float(&ntctemp_lookuptable[i]); //highest interval value
	if(i==0)
		mint = maxt;
	else
		mint = pgm_read_float(&ntctemp_lookuptable[i-1]); //smallest interval value

	//do interpolation
	a = a-ntctemp_lookupadcstep;
	t = mint + ((maxt-mint)/ntctemp_lookupadcstep) * (adcvalue-a);

	return t;
}
#endif


