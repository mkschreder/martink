/**
	Portable buffered uart driver

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

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef CONFIG_HAVE_UART5
#define UART_COUNT 6
#elif CONFIG_HAVE_UART4
#define UART_COUNT 5
#elif CONFIG_HAVE_UART3
#define UART_COUNT 4
#elif CONFIG_HAVE_UART2
#define UART_COUNT 3
#elif CONFIG_HAVE_UART1
#define UART_COUNT 2
#elif CONFIG_HAVE_UART0
#define UART_COUNT 1
#else 
#define UART_COUNT 0
#endif

serial_dev_t /*@null@*/ uart_get_serial_interface(uint8_t dev);

#ifdef __cplusplus
}
#endif
