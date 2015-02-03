#pragma once

#include <arch/time.h>

void tsc_init(void); 
timestamp_t tsc_read(void); 
timestamp_t tsc_ticks_to_us(timestamp_t ticks); 
timestamp_t tsc_us_to_ticks(timestamp_t us); 
