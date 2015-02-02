/*
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#pragma once 

#include "config.h"

#define wrap_pi(x) (x < -M_PI ? x+M_PI*2 : (x > M_PI ? x - M_PI*2: x))

inline long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#define constrain(x, a, b) (((x) < (a))?(a):(((x) > (b))?(b):(x)))
/*
static inline long constrain(long x, long a, long b){
	if(x < a) return a; 
	if(x > b) return b; 
	return x; 
}*/
/*
static inline float constrain_f(float x, float a, float b){
	if(x < a) return a; 
	if(x > b) return b; 
	return x; 
}*/

/*
#define assert(condition) ({\
	(condition)?(1):({uart0_printf("ERROR: assertion failed in %s line %d\n", __FILE__, __LINE__); while(1);});\
})
*/

#define container_of(ptr, type, member) ({                      \
        const __typeof__( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

//#define __init //__attribute__((constructor))

// wraps degrees around a circle
#define wrap_180(x) (x < -180 ? x+360 : (x > 180 ? x - 360: x))

#ifdef CONFIG_HAVE_UART
	#define kprintf(a, ...) uart0_printf(a, ##__VA_ARGS__) 
	
	#ifdef CONFIG_DEBUG
		#define kdebug(a, ...) {} //uart0_printf(a, ##__VA_ARGS__) 
	#else
		#define kdebug(a, ...) {} //uart0_printf(a, ##__VA_ARGS__) 
	#endif
#else 
	#define kprintf(a, ...) {}
	#define kdebug(a, ...) {}
#endif
