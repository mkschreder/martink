#pragma once

/** 
 * 	Author: Martin K. Schröder 
 *  Date: 2014
 * 
 * 	info@fortmax.se
 */

#ifdef __cplusplus
extern "C" {
#endif

/** Macros that one should use to correctly deal with timeouts **/


void time_init(void);

// returns current number of clock cycles (will overflow!)
timestamp_t tsc_read(void); 

// converts a value in microseconds to number of clock ticks
timestamp_t time_us_to_clock(timestamp_t us);
timestamp_t time_clock_to_us(timestamp_t clock); 
timestamp_t time_clock_since(timestamp_t clock); 

#ifdef __cplusplus
}
#endif
