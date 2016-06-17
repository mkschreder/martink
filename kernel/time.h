#pragma once 

#include "types.h"

extern long long tsc_read(void); 
extern long long tsc_ticks_to_us(long long ticks); 
extern long long tsc_us_to_ticks(long long us); 

#define timestamp_init() (tsc_init())
#define timestamp_now() (tsc_read())

#define timestamp_ticks_to_us(ticks) tsc_ticks_to_us(ticks)

//timestamp_t timestamp_ticks_to_us(timestamp_t ticks);
timestamp_t timestamp_us_to_ticks(timestamp_t us);

#define timestamp_before(unknown, known) (((timestamp_t)(unknown) - (timestamp_t)(known)) < 0)
#define timestamp_after(a,b) timestamp_before(b, a)

timestamp_t timestamp_from_now_us(timestamp_t us);

// timeout expired: can handle overflow of timer correctly
uint8_t timestamp_expired(timestamp_t timeout);

void timestamp_delay_us(timestamp_t usec);

#define delay_us(usec) timestamp_delay_us(usec)
#define delay_ms(msec) delay_us(((msec) * 1000UL))

timestamp_t timestamp_ticks_since(timestamp_t timestamp) ;

#define wait_on_us(cond, timeout) {timestamp_t ts = timeout; while((cond) && ts--) delay_us(1); }

void time_init(void); 


