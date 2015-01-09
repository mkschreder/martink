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

#include "autoconf.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern FILE *uart0_fd;

void uart0_init(uint32_t baudrate);

size_t uart0_puts(const char *s );
uint16_t uart0_printf(const char *fmt, ...); 

//#define uart0_printf(format, ...) fprintf((uart0_fd), format, ## __VA_ARGS__)
//#define uart0_puts(str) fputs(str, uart0_fd)

#define uart1_init(baud) uart1_init_default(baud)

#define UART_PARITY_ERROR			SERIAL_PARITY_ERROR
#define UART_FRAME_ERROR      SERIAL_FRAME_ERROR              
#define UART_OVERRUN_ERROR    SERIAL_OVERRUN_ERROR              
#define UART_BUFFER_FULL  		SERIAL_BUFFER_FULL             
#define UART_NO_DATA          SERIAL_NO_DATA              

serial_dev_t uart_get_serial_interface(uint8_t dev);

#ifdef __cplusplus
}
#endif
