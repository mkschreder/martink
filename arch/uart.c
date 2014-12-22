/**
	Portable uart driver code

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

#include <arch/soc.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <static_cbuf.h>

#define UART_DEFAULT_BAUDRATE 38400

#ifdef CONFIG_AVR
static FILE _fd; 
FILE *uart0_fd = &_fd;

int _uart0_fd_get(FILE *fd){
	return uart0_getc();
}

int _uart0_fd_put(char data, FILE *fd){
	return uart0_putc(data);
}
#endif

void uart0_init(uint32_t baudrate) {
	uart0_init_default(baudrate);
	//uart0_fd->get = _uart0_fd_get;
	//uart0_fd->put = _uart0_fd_put; 
}

/// *************************
/// UART0 interface functions
/// *************************

size_t _uart0_waiting(struct serial_interface *self){
	return uart0_waiting(); 
}

void _uart0_flush(struct serial_interface *self){
	uart0_flush(); 
}

uint16_t _uart0_getc(struct serial_interface *self){
	return uart0_getc(); 
}

uint16_t _uart0_putc(struct serial_interface *self, unsigned char data){
	return uart0_putc(data);
}

size_t _uart0_putn(struct serial_interface *self, const uint8_t *s, size_t c)
{
	size_t t = c; 
	while (c--) 
		uart0_putc(*s++);
	return t; 
}

size_t _uart0_getn(struct serial_interface *self, uint8_t *s, size_t c)
{
	size_t t = 0; 
	while (t < c) {
		uint16_t d = uart0_getc();
		if(d == UART_NO_DATA) return t; 
		t++; 
	}
	return t; 
}

size_t uart0_puts(const char *s )
{
	size_t count = 0; 
	while (*s) {
		uart0_putc(*s++);
		count++; 
	}
	return count; 
}

uint16_t uart0_printf(const char *fmt, ...){
	char buf[64]; 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf(buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	uart0_puts(buf);
	return n; 
}

static void __init uart_init(void){
	kdebug("UART: starting interfaces: "); 
#ifdef CONFIG_HAVE_UART0
	uart0_init(UART_DEFAULT_BAUDRATE); kdebug("0 "); 
#endif
#ifdef CONFIG_HAVE_UART1
	uart1_init(UART_DEFAULT_BAUDRATE); kdebug("1 "); 
#endif
#ifdef CONFIG_HAVE_UART2
	uart2_init(UART_DEFAULT_BAUDRATE); kdebug("2 "); 
#endif
#ifdef CONFIG_HAVE_UART3
	uart3_init(UART_DEFAULT_BAUDRATE); kdebug("3 "); 
#endif
	kdebug("\n"); 
}
