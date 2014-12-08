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

// default for strings stored in flash
#define PSTR(a) (a)

#ifndef CONFIG_UART0_NAME
#define CONFIG_UART0_NAME uart0
#endif
#ifndef CONFIG_UART1_NAME
#define CONFIG_UART1_NAME uart1
#endif

// generic uart interface
#ifdef CONFIG_HAVE_UART0
#define uart0_init(baud) PFCALL(CONFIG_UART0_NAME, init, baud)
#define uart0_printf(fmt, args...) PFCALL(CONFIG_UART0_NAME, printf, fmt, args) 
#define uart0_puts(fmt) PFCALL(CONFIG_UART0_NAME, puts, fmt) 
#define uart0_putc(ch) PFCALL(CONFIG_UART0_NAME, putc, ch)

#define uart_init uart0_init
#define uart_printf uart0_printf
#define uart_puts uart0_puts
#define uart_putc uart0_putc
#endif

#ifdef CONFIG_HAVE_UART1
#define uart1_init(baud) PFCALL(CONFIG_UART1_NAME, init, baud)
#define uart1_printf(fmt, args...) PFCALL(CONFIG_UART1_NAME, printf, fmt, args) 
#define uart1_puts(fmt) PFCALL(CONFIG_UART1_NAME, puts, fmt) 
#define uart1_putc(ch) PFCALL(CONFIG_UART1_NAME, putc, ch)
#endif

