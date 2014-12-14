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

#include "autoconf.h"

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

// used for generating references to static methods
#define PFNAME2(P, F) __##P##_##F##__
#define PFNAME(P, F) PFNAME2(P, F)

// used for generating calls to static device methods
#define PFCALL2(P, F, args...) __##P##_##F##__( args )
#define PFCALL(P, F, args...) PFCALL2(P, F, args )

// used for declaring driver functions
#define PFDECL(P, F, args...) PFCALL(P, F, args )

#define __init __attribute__((constructor))

#include "interface.h"
#include "static_cbuf.h"

#ifdef CONFIG_ATMEGA328P
#include "avr/m328p.h"
#endif

#ifdef CONFIG_ARM
#include "arm/arm.h"
#endif

#ifdef CONFIG_NATIVE
#include "native/native.h"
#endif

#include "time.h"
#include "uart.h"
#include "twi.h"
#include "spi.h"
#include "gpio.h"

/*
struct d_char {
	void 			(*const putc)(uint8_t ch); 
	uint16_t 	(*const getc)(void); 
	size_t 		(*const write)(const uint8_t *data, size_t size); 
	size_t 		(*const read)(uint8_t *data, size_t max_count); 
}; 

struct d_block {
	size_t 		(*const read)(size_t where, uint8_t *data, size_t max_count); 
	size_t 		(*const write)(size_t where, const uint8_t *data, size_t max_count); 
}; 

struct d_spi {
	void 			(*const init)(void); 
	uint8_t 	(*const writereadbyte)(uint8_t ch); 
}; 

#define DEV_CHAR_DEVICE(name) {\
	.putc = PFNAME(name, putc), \
	.getc = PFNAME(name, getc), \
	.write = PFNAME(name, write), \
	.read = PFNAME(name, read)\
}

#define DEV_SPI_DEVICE(device) {\
	.init = PFNAME(device, init), \
	.writereadbyte = PFNAME(device, writereadbyte)\
}

#define DEV_BLOCK_DEVICE(name) {\
	.read = PFNAME(name, read), \
	.write = PFNAME(name, write) \
}

struct d_soc {
	#ifdef CONFIG_HAVE_UART0
	const struct d_char uart0; 
	#endif
	#ifdef CONFIG_HAVE_UART1
	const struct d_char uart1; 
	#endif
	#ifdef CONFIG_HAVE_UART2
	const struct d_char uart2; 
	#endif
	#ifdef CONFIG_HAVE_UART3
	const struct d_char uart3; 
	#endif
	#ifdef CONFIG_HAVE_SPI0
	const struct d_spi spi0; 
	#endif
	#ifdef CONFIG_HAVE_SPI1
	const struct d_spi spi1; 
	#endif
	#ifdef CONFIG_HAVE_TWI0
	const struct d_block twi0; 
	#endif
	#ifdef CONFIG_HAVE_TWI1
	const struct d_block twi1; 
	#endif
}; 

//const struct d_soc *main_cpu(void); 

extern const struct d_soc cpu; 
*/

#ifndef PROGMEM
#define PROGMEM
#define PSTR(a) (a)
#define pgm_read_byte(a) (*a)
#define pgm_read_word(a) (*((short int*)a))
#define pgm_read_word(a) (*((short int*)a))
#endif

#ifdef __cplusplus
}
#endif
