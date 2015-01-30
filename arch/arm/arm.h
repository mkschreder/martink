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

#define sei() {}

#ifdef CONFIG_STM32F103
#include "stm32/stm32.h"
#endif

#ifdef CONFIG_AT91SAM3
#include "sam3/sam.h"
#include "at91sam3xe.h"
#endif
/*
#include "uart.h"
#include "twi.h"
#include "spi.h"
#include "gpio.h"
#include "time.h"
#include "adc.h"
*/
#include "memory.h"

typedef char * uint_farptr_t; 
#define strcpy_PF(a, b) strcpy(a, b)
#define strcmp_PF(a, b) strcmp(a, b)
#define memcpy_PF(a, b, s) memcpy(a, b, s)
#define PSTR(a) (a)
//#define pgm_read_word(a) (*(uint16_t*)a)
