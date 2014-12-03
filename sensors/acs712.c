/*
acs712 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


/*
 * get the current
 * current = (V - Vref/2) / sensitivity
 */

#include "acs712.h"

double acs712_getcurrent(double voltagein, double adcvref) {
	return (double)((double)(voltagein - (double)(adcvref/(double)2)) / (double)ACS712_SENSITIVITY);
}


