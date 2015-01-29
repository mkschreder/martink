#pragma once

#define TSC_TICKS_PER_US 0.001

#include "../time.h"

#define tsc_init(void) (\
	TimeTick_Configure(SystemCoreClock)\
)

static inline timestamp_t tsc_read(void){
	#warning "returning zero tick count every time!"
	return 0; 
	//return GetTickCount();
}

static inline timestamp_t tsc_us_to_ticks(timestamp_t us) {
	return ((timestamp_t)(TSC_TICKS_PER_US * ((timestamp_t)us)));
}

static inline timestamp_t tsc_ticks_to_us(timestamp_t ticks){
	return (((timestamp_t)ticks) / TSC_TICKS_PER_US);
}

#define tsc_reset(void) do {} while(0); 
