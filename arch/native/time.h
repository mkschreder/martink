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

// returns current number of clock cycles (will overflow!)
timestamp_t tsc_read(void); 

// converts a value in microseconds to number of clock ticks
timestamp_t tsc_us_to_ticks(timestamp_t us);
timestamp_t tsc_ticks_to_us(timestamp_t clock); 

#ifdef __cplusplus
}
#endif
