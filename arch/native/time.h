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
timeout_t time_get_clock(void); 

// converts a value in microseconds to number of clock ticks
timeout_t time_us_to_clock(timeout_t us);
timeout_t time_clock_to_us(timeout_t clock); 
timeout_t time_clock_since(timeout_t clock); 

#ifdef __cplusplus
}
#endif
