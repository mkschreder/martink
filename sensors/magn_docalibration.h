/*
magnetometer calibration functions 01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

//use this module in association with the python script provided

#if MAGN_DOCALIBRATION == 1

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

/*
 * do calibration
 */
void magncal_docalibrationclient(void(*cputc)(unsigned char), unsigned int (*cgetc)()) {
	int16_t mxraw = 0;
	int16_t myraw = 0;
	int16_t mzraw = 0;
	for(;;) {
		//wait for the input request
		while(cgetc() != 0x22);
		//get raw data
		magncal_getrawdata(&mxraw, &myraw, &mzraw);
		//send raw data
		cputc(mxraw);
		cputc(mxraw>>8);
		cputc(myraw);
		cputc(myraw>>8);
		cputc(mzraw);
		cputc(mzraw>>8);
		cputc('\n');
	}
}
#endif
