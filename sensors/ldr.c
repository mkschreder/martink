/*
LDR 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include <math.h>

#include "ldr.h"

/*
 * return the lux
 *
 * "adcresistance" resistance measured on pin (ohm)
 *
 * lux = A*((R/B)^C)
 */
#define LDR_A 9000
#define LDR_B 165
#define LDR_C -1.65
unsigned long ldr_getlux(long adcresistance) {
	double a = adcresistance/(double)LDR_B;
	return (long) ((double)LDR_A*(pow((double)a, (double)LDR_C)));
}
