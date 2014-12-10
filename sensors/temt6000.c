/*
temt6000 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>

#include "temt6000.h"

/*
 * get lux from voltage
 */
double temt6000_getlux(double vol) {
	//suppose we read the voltage from the emitter of temt6000, with a 10k pulldown resistor connected to gnd
	//V[V] = I[A]*R[ohm] = I[uA] * 1[A]/1000000[µA] * 10000[ohm] = I[uA] * 1[A]/1[uA] * 0.01[ohm]
	//1[uA] = V[V] / 0.01[ohm] = 100 * V[V/ohm]
	//the curve from datasheet seems y=2x+0, so lux = 2 * I[uA] = 2 * 100 * V[V/ohm]
	return 2*(100*vol);
}


