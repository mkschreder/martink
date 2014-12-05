/** 
 * 	Author: Martin K. Schröder 
 *  Date: 2014
 * 
 * 	info@fortmax.se
 */

#include <stdint.h>
#include <time.h>
#include <sys/timeb.h>

#include "time.h"

// update this for correct amount of ticks in microsecon
#define TICKS_PER_US 1

void time_init(void){
	
}

void time_reset(void){
	
}

timeout_t time_us_to_clock(timeout_t us){
	return TICKS_PER_US * us; 
}

timeout_t time_clock_to_us(timeout_t clock){
	return clock / TICKS_PER_US; 
}

timeout_t time_get_clock(void){
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (timeout_t)ts.tv_sec * 1000000LL + (timeout_t)ts.tv_nsec / 1000LL;
	/*unsigned a, d; 
	asm volatile("rdtsc" : "=a" (a), "=d" (d)); 
	return (((unsigned long long)a) | (((unsigned long long)d) << 32)); */
	//return TCNT1 + _timer1_ovf * 65535;
}

void time_delay(timeout_t usec){
	timeout_t t = timeout_from_now(usec); 
	while(!timeout_expired(t)); 
}

timeout_t time_clock_since(timeout_t clock){
	return time_get_clock() - clock; 
}
