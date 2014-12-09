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

#ifdef __cplusplus
extern "C" {
#endif

// these should be moved somewhere. This is the way to do configurable
// c interfaces using macros. It is used by all generic interface headers. 
#define PFCALL2(P, F, args...) __##P##_##F##__(args)
#define PFCALL(P, F, args...) PFCALL2(P, F, args)


#include "time.h"
#include "uart.h"
#include "twi.h"

#ifdef CONFIG_ATMEGA328P
#include "avr/m328p.h"
#endif

#ifdef CONFIG_ARM
#include "arm/arm.h"
#endif

#ifdef CONFIG_NATIVE
#include "native/native.h"
#endif

#ifndef PROGMEM
#define PROGMEM
#define pgm_read_byte(a) (*a)
#endif

#ifdef __cplusplus
}
#endif
