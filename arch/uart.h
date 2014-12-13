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

struct uart_interface {
	/// getc reads one character from the interface
	/// returns UART_NO_DATA if none available
	unsigned int 	(*getc)(void); 
	/// putc writes a character into the interface
	/// waits for a timeout of 1-2 bytes if output buffer is full 
	/// discards data if tx buffer is full and no data can be sent
	/// must return reasonably immidiately. 
	void 					(*putc)(unsigned char ch); 
	/// putn writes a buffer into the interface
	/// returns number of bytes written
	/// should return only when data has been at least copied into internal buffer of the driver
	/// is not required to wait until data has been transmitted on wire
	size_t				(*putn)(const char *data, size_t max_sz);
	/// reads a number of characters from device
	/// returns number of bytes read
	/// returns always without waiting for more data to arrive
	/// always works with internal rx buffer of the interface
	size_t				(*getn)(const char *data, size_t max_sz);
	/// flushes the output buffer and returns only when completed
	/// must ensure that all data has been sent on wire
	void					(*flush)(void);
	/// returns current number of bytes available for reading
	size_t 				(*waiting)(void);
}; 

#define UART_DEVICE_INTERFACE(uart) (struct uart_interface){\
	.getc = PFNAME(uart, getc), \
	.putc = PFNAME(uart, putc), \
	.getn = PFNAME(uart, getn), \
	.putn = PFNAME(uart, putn), \
	.flush = PFNAME(uart, flush), \
	.waiting = PFNAME(uart, waiting), \
}

//#define uart_init(baud) PFCALL(CONFIG_UART0_NAME, init, baud)
#define uart_printf(fmt, args...) PFCALL(CONFIG_UART0_NAME, printf, fmt, args) 
#define uart_puts(fmt) PFCALL(CONFIG_UART0_NAME, puts, fmt) 
#define uart_putn(data, sz) PFCALL(CONFIG_UART0_NAME, putn, data, sz)
#define uart_getn(data, sz) PFCALL(CONFIG_UART0_NAME, getn, data, sz) 
#define uart_putc(ch) PFCALL(CONFIG_UART0_NAME, putc, ch)
#define uart_getc(ch) PFCALL(CONFIG_UART0_NAME, getc)

//#define printf(fmt, args...) uart_printf(uart0, PSTR(fmt), args)
//#define puts(str) uart_printf(uart0, str)

// generic uart interface
#ifdef CONFIG_HAVE_UART0
#define uart0_printf(fmt, args...) PFCALL(CONFIG_UART0_NAME, printf, fmt, args) 
#define uart0_puts(fmt) PFCALL(CONFIG_UART0_NAME, puts, fmt) 
#define uart0_putn(data, sz) PFCALL(CONFIG_UART0_NAME, putn, data, sz)
#define uart0_getn(data, sz) PFCALL(CONFIG_UART0_NAME, getn, data, sz) 
#define uart0_putc(ch) PFCALL(CONFIG_UART0_NAME, putc, ch)
#define uart0_getc() PFCALL(CONFIG_UART0_NAME, getc)
#define uart0_flush() PFCALL(CONFIG_UART0_NAME, flush)
#define uart0_waiting() PFCALL(CONFIG_UART0_NAME, waiting)
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

extern void PFDECL(CONFIG_UART0_NAME, init, unsigned int baudrate);
extern uint16_t PFDECL(CONFIG_UART0_NAME, waiting, void); 
extern unsigned int PFDECL(CONFIG_UART0_NAME, getc, void);
extern void PFDECL(CONFIG_UART0_NAME, putc, unsigned char data);
extern void PFDECL(CONFIG_UART0_NAME, puts, const char *s );
extern size_t PFDECL(CONFIG_UART0_NAME, putn, const char *s, size_t c); 
extern size_t PFDECL(CONFIG_UART0_NAME, getn, const char *s, size_t c); 
//extern void PFDECL(CONFIG_UART0_NAME, puts_p, const char *s );
extern uint16_t PFDECL(CONFIG_UART0_NAME, printf, const prog_char *fmt, ...);
