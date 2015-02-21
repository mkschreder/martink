/**
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

#ifdef __cplusplus
extern "C" {
#endif 

#include "config.h"
#include <inttypes.h>
#include <string.h>
#include <arch/soc.h>
#include "util.h"
#include <list.h>

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
__extension__ typedef int __guard __attribute__((mode (__DI__)));

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *);
extern "C" void __cxa_pure_virtual(void);
#else 
	typedef int __guard; 
#endif

struct libk_thread {
	struct list_head list; 
	char (*proc)(struct pt *self); 
	struct pt pt; 
}; 

void libk_create_thread(struct libk_thread *self); 
void libk_schedule(void); 

#define LIBK_THREAD(c, func) static struct libk_thread __thread##c##; \
	static PT_THREAD(func(struct pt *)); \
	__attribute__((constructor)) void __init__thread##c##(void){ libk_create_thread(&__thread##c##, func); } \
	static PT_THREAD(func(struct pt *pt))
	
#define LIBK_THREAD(func) _LIBK_THREAD(__COUNTER__, func)

//#include "wiinunchuck.h"
