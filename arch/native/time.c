/** 
 * 	Author: Martin K. Schröder 
 *  Date: 2014
 * 
 * 	info@fortmax.se
 */

#include <stdint.h>
#include <time.h>
#include <sys/timeb.h>
#if __STDC_VERSION__ >= 199901L
#define _XOPEN_SOURCE 600
#else
#define _XOPEN_SOURCE 500
#endif /* __STDC_VERSION__ */

#include <time.h>

#include "time.h"

// update this for correct amount of ticks in microsecon
#define TICKS_PER_US 1

timestamp_t tsc_us_to_ticks(timestamp_t us){
	return TICKS_PER_US * us; 
}

timestamp_t tsc_ticks_to_us(timestamp_t clock){
	return clock / TICKS_PER_US; 
}

timestamp_t tsc_read(void){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (timestamp_t)ts.tv_sec * 1000000LL + (timestamp_t)ts.tv_nsec / 1000LL;
	/*unsigned a, d; 
	asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
	return (((unsigned long long)a) | (((unsigned long long)d) << 32)); */
	//return TCNT1 + _timer1_ovf * 65535;
}

/*
timestamp_t time_clock_since(timestamp_t clock){
	return time_get_clock() - clock; 
}*/
