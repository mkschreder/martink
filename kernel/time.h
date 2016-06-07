#pragma once 

#include "types.h"

extern timestamp_t tsc_read(void); 
extern timestamp_t tsc_ticks_to_us(timestamp_t t); 
extern timestamp_t tsc_us_to_ticks(timestamp_t t); 

#define timestamp_init() (tsc_init())
#define timestamp_now() (tsc_read())

static inline timestamp_t timestamp_ticks_to_us(timestamp_t ticks){ return tsc_ticks_to_us(ticks); }
static inline timestamp_t timestamp_us_to_ticks(timestamp_t us){ return tsc_us_to_ticks(us); }

#define timestamp_before(unknown, known) (((timestamp_t)(unknown) - (timestamp_t)(known)) < 0)
#define timestamp_after(a,b) timestamp_before(b, a)

static inline timestamp_t timestamp_from_now_us(timestamp_t us) { 
	return tsc_read() + timestamp_us_to_ticks(us); 
}

// timeout expired: can handle overflow of timer correctly
static inline uint8_t timestamp_expired(timestamp_t timeout){
	return timestamp_after(tsc_read(), timeout); 
}

void timestamp_delay_us(timestamp_t usec);

#define delay_us(usec) timestamp_delay_us(usec)
#define delay_ms(msec) delay_us(((msec) * 1000UL))

timestamp_t timestamp_ticks_since(timestamp_t timestamp) ;

#define wait_on_us(cond, timeout) {timestamp_t ts = timeout; while((cond) && ts--) delay_us(1); }

void time_init(void); 


