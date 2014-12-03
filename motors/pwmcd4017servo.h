/*
pwmcd4017 servo lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#ifndef PWMCD4017SERVO_H_
#define PWMCD4017SERVO_H_

#include <stdio.h>
#include <avr/io.h>

#include "pwmcd4017.h"

//PWMCD4017_SERVOFREQTIME = PWMCD4017_DELAYTIMEMS/(PWMCD4017_FREQUS*0.001)
//PWMCD4017_DELAYTIMEMS is the frequency of the servo, for 50Hz, 1/50 = 20ms
//PWMCD4017_SERVODEGREE1MSSTEP is the interrupt it takes to make 1ms step
//1ms_step = 1ms / (PWMCD4017_FREQUS*0.001)
//2ms_step = 2ms / (PWMCD4017_FREQUS*0.001)
//PWMCD4017_SERVODEGREEFACTOR is the degree factor
//degreefactor = 2ms_step-1ms_step / 180
//degreecount = 1ms_step + PWMCD4017_SERVODEGREEFACTOR * degree
//PWMCD4017_SERVODEGREEPRECISION is the precision for the servo rotation
//degreeprecision = 180 / 1ms_step
#if PWMCD4017_HIGHACCU == 1
//PWMCD4017_FREQUS is 1us here
#define PWMCD4017_SERVOFREQTIME 20000
#define PWMCD4017_SERVODEGREE1MSSTEP 1000
#define PWMCD4017_SERVODEGREEFACTOR 5.56
#define PWMCD4017_SERVODEGREEPRECISION 0.18
#else
#define PWMCD4017_SERVOFREQTIME 1600
#define PWMCD4017_SERVODEGREE1MSSTEP 80
#define PWMCD4017_SERVODEGREEFACTOR 0.45
//180 / 80 = 2.25 degree of precision
#define PWMCD4017_SERVODEGREEPRECISION 2.25
#endif

//PWMCD4017_SERVOMAX is the max number of servos
//the last pin of every chip is for the deadtime pulse
//so for every chip we have PWMCD4017_CHIPPIN-1 servos max
//max servos available are PWMCD4017_CHIP *(PWMCD4017_CHIPPIN-1)
#define PWMCD4017_SERVOMAX PWMCD4017_CHIP *(PWMCD4017_CHIPPIN-1)

/*
 * convert degree to ms
 */
#define pwmcd4017_servogetdelay(deg) (deg >= 0 || deg <= 180 ? PWMCD4017_SERVODEGREE1MSSTEP + (PWMCD4017_SERVODEGREEFACTOR * (double)deg) : 0 )

extern void pwmcd4017_servosetdeadtime();
extern void pwmcd4017_servoinit();
extern void pwmcd4017_servoset(uint8_t id, uint8_t deg);

/*
 * set servo dead time
 * the last clock should be a deadtime-clock
 */
void pwmcd4017_servosetdeadtime() {
	uint8_t i,j = 0;
	for(i=0; i<PWMCD4017_CHIP; i++) {
		uint16_t deadtime = PWMCD4017_SERVOFREQTIME;
		for(j=0; j<PWMCD4017_CHIPPIN-1; j++) {
			deadtime -= pwmcd4017_array[i].clock[j];
		}
		pwmcd4017_array[i].clock[PWMCD4017_CHIPPIN-1] = deadtime;
	}
}

/*
 * init all servos to 0 degree
 */
void pwmcd4017_servoinit() {
	uint8_t i = 0;
	for(i=0; i<PWMCD4017_CHIPPIN*PWMCD4017_CHIP; i++) {
		pwmcd4017_set(i, pwmcd4017_servogetdelay(0));
	}
	pwmcd4017_servosetdeadtime(); //set deadtime
}

/*
 * set a servo degree
 */
void pwmcd4017_servoset(uint8_t id, uint8_t deg) {
	if(id > PWMCD4017_SERVOMAX)
		return;
	//reconvert id to a valid pin (remember that the pin 9 of every chip is for deadtime pulse)
	uint8_t t1 = id%(PWMCD4017_CHIPPIN-1);
	uint8_t t2 = (int)(id/(PWMCD4017_CHIPPIN-1));
	id = t1 + t2*((PWMCD4017_CHIPPIN-1)+1);
	pwmcd4017_set(id, pwmcd4017_servogetdelay(deg));
	pwmcd4017_servosetdeadtime();
}

#endif

