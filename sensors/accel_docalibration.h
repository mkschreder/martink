/*
accellerometer calibration functions 01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

//use this module in association with the python script provided

#if ACCEL_DOCALIBRATION == 1

#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <string.h>

/*
 * do calibration
 */
void accelcal_docalibrationclient(void(*cputc)(unsigned char), unsigned int (*cgetc)()) {
    int16_t axraw = 0;
    int16_t ayraw = 0;
    int16_t azraw = 0;
    for(;;) {
    	//wait for the input request
    	while(cgetc() != 0x20);
    	//get raw data
    	accelcal_getrawdata(&axraw, &ayraw, &azraw);
    	//send raw data
    	cputc(axraw);
    	cputc(axraw>>8);
    	cputc(ayraw);
		cputc(ayraw>>8);
		cputc(azraw);
		cputc(azraw>>8);
		cputc('\n');
    }
}
#endif
