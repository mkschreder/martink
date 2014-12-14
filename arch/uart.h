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

#define UART_DEVICE_INTERFACE(uart) CONSTRUCT_SERIAL_INTERFACE(uart)

DECLARE_SERIAL_INTERFACE(CONFIG_UART0_NAME); 

extern void PFDECL(CONFIG_UART0_NAME, init, uint32_t baudrate);
extern size_t PFDECL(CONFIG_UART0_NAME, puts, const char *str); 
extern uint16_t PFDECL(CONFIG_UART0_NAME, printf, const char *fmt, ...);

//#define uart_init(baud) PFCALL(CONFIG_UART0_NAME, init, baud)
#define uart_printf(fmt, args...) PFCALL(CONFIG_UART0_NAME, printf, 0, fmt, args) 
#define uart_puts(fmt) PFCALL(CONFIG_UART0_NAME, puts, 0, fmt) 
#define uart_putn(data, sz) PFCALL(CONFIG_UART0_NAME, putn, 0, data, sz)
#define uart_getn(data, sz) PFCALL(CONFIG_UART0_NAME, getn, 0, data, sz) 
#define uart_putc(ch) PFCALL(CONFIG_UART0_NAME, putc, 0, ch)
#define uart_getc(ch) PFCALL(CONFIG_UART0_NAME, getc, 0)

//#define printf(fmt, args...) uart_printf(uart0, PSTR(fmt), args)
//#define puts(str) uart_printf(uart0, str)

// generic uart interface
#ifdef CONFIG_HAVE_UART0
#define uart0_printf(fmt, args...) PFCALL(CONFIG_UART0_NAME, printf, fmt, args) 
#define uart0_puts(fmt) PFCALL(CONFIG_UART0_NAME, puts, fmt) 

#define uart0_putn(data, sz) PFCALL(CONFIG_UART0_NAME, putn, 0, data, sz)
#define uart0_getn(data, sz) PFCALL(CONFIG_UART0_NAME, getn, 0, data, sz) 
#define uart0_putc(ch) PFCALL(CONFIG_UART0_NAME, putc, 0, ch)
#define uart0_getc() PFCALL(CONFIG_UART0_NAME, getc, 0)
#define uart0_flush() PFCALL(CONFIG_UART0_NAME, flush, 0)
#define uart0_waiting() PFCALL(CONFIG_UART0_NAME, waiting, 0)
#endif

#ifdef CONFIG_HAVE_UART1
#define uart1_init(baud) PFCALL(CONFIG_UART1_NAME, init, baud)
//#define uart1_printf(fmt, args...) PFCALL(CONFIG_UART1_NAME, printf, fmt, args) 
//#define uart1_puts(fmt) PFCALL(CONFIG_UART1_NAME, puts, fmt) 
#define uart1_putc(ch) PFCALL(CONFIG_UART1_NAME, putc, ch)
#define uart1_getc() PFCALL(CONFIG_UART0_NAME, getc)
#define uart1_putn(data, sz) PFCALL(CONFIG_UART1_NAME, putn, data, sz)
#define uart1_getn(data, sz) PFCALL(CONFIG_UART1_NAME, getn, data, sz) 
#endif

/** Size of the circular receive buffer, must be power of 2 */
#ifndef CONFIG_UART0_TX_BUF_SIZE
#define CONFIG_UART0_TX_BUF_SIZE 32
#endif
/** Size of the circular transmit buffer, must be power of 2 */
#ifndef CONFIG_UART0_RX_BUF_SIZE
#define CONFIG_UART0_RX_BUF_SIZE 32
#endif

#define UART_TX_BUFFER_SIZE CONFIG_UART0_TX_BUF_SIZE
#define UART_RX_BUFFER_SIZE CONFIG_UART0_RX_BUF_SIZE

#define UART_PARITY_ERROR			SERIAL_PARITY_ERROR
#define UART_FRAME_ERROR      SERIAL_FRAME_ERROR              /* Framing Error by UART       */
#define UART_OVERRUN_ERROR    SERIAL_OVERRUN_ERROR              /* Overrun condition by UART   */
#define UART_BUFFER_FULL  		SERIAL_BUFFER_FULL             /* receive ringbuffer overflow */
#define UART_NO_DATA          SERIAL_NO_DATA              /* no receive data available   */
