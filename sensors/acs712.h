/*
acs712 lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef ACS712_H_
#define ACS712_H_

//defined sensitivity
#define ACS712_SENSITIVITY5 0.185
#define ACS712_SENSITIVITY20 0.100
#define ACS712_SENSITIVITY30 0.066

//setup sensitivity
#define ACS712_SENSITIVITY ACS712_SENSITIVITY30

extern double acs712_getcurrent(double voltagein, double adcvref);

#endif
