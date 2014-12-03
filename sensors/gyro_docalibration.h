/*
gyroscope calibration functions 01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

//use this module in association with the python script provided

#if GYRO_DOCALIBRATION == 1

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

/*
 * do calibration
 */
void gyrocal_docalibrationclient(void(*cputc)(unsigned char), unsigned int (*cgetc)()) {
    int16_t gxraw = 0;
    int16_t gyraw = 0;
    int16_t gzraw = 0;
	for(;;) {
		//wait for the input request
		while(cgetc() != 0x21);
		//get raw data
		gyrocal_getrawdata(&gxraw, &gyraw, &gzraw);
		//send raw data
		cputc(gxraw);
		cputc(gxraw>>8);
		cputc(gyraw);
		cputc(gyraw>>8);
		cputc(gzraw);
		cputc(gzraw>>8);
		cputc('\n');
	}
}
#endif
