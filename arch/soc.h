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

#include "config.h"

#include "types.h"

#include "../util.h"

#include <inttypes.h>
#include <stddef.h>


#ifdef __cplusplus
extern "C" {
#endif


#ifndef _BV
#define _BV(a) (1 << (a))
#endif

// these should be moved somewhere. This is the way to do configurable
// c interfaces using macros. It is used by all generic interface headers. 
/*
// used for generating references to static methods
#define PFNAME2(P, F) __##P##_##F##__
#define PFNAME(P, F) PFNAME2(P, F)

// used for generating calls to static device methods
#define PFCALL2(P, F, args...) __##P##_##F##__( args )
#define PFCALL(P, F, args...) PFCALL2(P, F, args )

// used for declaring driver functions
#define PFDECL(P, F, args...) PFCALL(P, F, args )
*/

#define initproc //__attribute__((constructor))

#ifdef CONFIG_AVR
#include "avr/mega.h"
#elif CONFIG_ARM
#include "arm/arm.h"
#elif CONFIG_NATIVE
#include "native/native.h"
#else 
#error "You have not chosen an architecture!"
#endif

#include "interface.h"
#include "static_cbuf.h"
#include <util/list.h>

#include "time.h"
#include "uart.h"
#include "twi.h"
#include "spi.h"
#include "gpio.h"
#include "pwm.h"

#ifndef PROGMEM
#define PROGMEM
#define PSTR(a) (a)
#define pgm_read_byte(a) (*a)
#define pgm_read_word(a) (*((const uint16_t*)a))
#define pgm_read_dword(a) (*((const uint32_t*)a))
#define pgm_read_float(a) (*((const float*)a))
#define pgm_streq(str, pstr) (strcmp(str, pstr) == 0)
#define pgm_snprintf(str, size, fmt, ...) snprintf(str, size, fmt, ##__VA_ARGS__)
#else
#define pgm_streq(str, pstr) (strcmp_P(str, pstr) == 0)
#define pgm_snprintf(str, size, fmt, ...) snprintf_P(str, size, fmt, ##__VA_ARGS__)
#endif

//void soc_init(void); 

/// architecture specific implementation of serial_printf
/// the implementation needs to be implemented separately because of 
/// differences in libc. Look in arch/syscalls.c
uint16_t serial_printf(serial_dev_t port, const char *fmt, ...);


struct libk_thread {
	struct list_head list; 
	char (*proc)(struct pt *self); 
	struct pt thread; 
	// debug
	long unsigned int time; 
	long unsigned int framecount, timecount; 
	const char *name; 
}; 

void libk_create_thread(struct libk_thread *self, char (*proc)(struct pt *self), const char *name); 
void libk_delete_thread(struct libk_thread *self); 
void libk_schedule(void); 
void libk_schedule_thread(struct pt *thread, void (*proc)(void *arg), void *arg); 
struct pt *libk_current_thread(void); 
void libk_run(void); 
uint32_t libk_get_fps(void); 
void libk_print_info(void); 

#define __LIBK_THREAD(c, func) static struct libk_thread __thread##c##__; \
	static PT_THREAD(func(struct pt *)); \
	__attribute__((constructor)) static void __init__thread##c##__(void){ libk_create_thread(&__thread##c##__, func, #func); } \
	static PT_THREAD(func(struct pt *pt))

#define _LIBK_THREAD(c, func) __LIBK_THREAD(c, func)
#define LIBK_THREAD(func) _LIBK_THREAD(__COUNTER__, func)

#ifdef __cplusplus
}
#endif
