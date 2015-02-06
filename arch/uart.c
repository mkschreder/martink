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

#ifdef CONFIG_HAVE_UART3
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


struct uart_device {
	uint8_t id; 
	struct serial_if *serial; 
}; 

struct uart_device _uart[1]; 

void uart0_init(uint32_t baudrate) {
	uart0_init_default(baudrate);
	//uart0_fd->get = _uart0_fd_get;
	//uart0_fd->put = _uart0_fd_put; 
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
/*
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

*/
#define GET_DEV(s, dev) \
	struct uart_device *dev = container_of((s), struct uart_device, serial)
	
uint16_t _uart_putc(serial_dev_t self, uint8_t ch){
	GET_DEV(self, dev);
	switch(dev->id){
		#define LABEL(id) case id: return uart##id##_putc(ch); break; 
		#ifdef CONFIG_HAVE_UART0
		LABEL(0)
		#endif
		#ifdef CONFIG_HAVE_UART1
		LABEL(1)
		#endif
		#ifdef CONFIG_HAVE_UART2
		LABEL(2)
		#endif
		#ifdef CONFIG_HAVE_UART3
		LABEL(3)
		#endif
		#undef LABEL
	}
	return 0; 
}

uint16_t _uart_getc(serial_dev_t self) {
	GET_DEV(self, dev);
	switch(dev->id){
		#define LABEL(id) case id: {int c = uart##id##_getc(); \
			if(c == UART_NO_DATA) return SERIAL_NO_DATA; \
			return c; break; }
		#ifdef CONFIG_HAVE_UART0
		LABEL(0)
		#endif
		#ifdef CONFIG_HAVE_UART1
		LABEL(1)
		#endif
		#ifdef CONFIG_HAVE_UART2
		LABEL(2)
		#endif
		#ifdef CONFIG_HAVE_UART3
		LABEL(3)
		#endif
		#undef LABEL
	}
	return -1; 
}

size_t _uart_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_uart_putc(self, *data++); 
	}
	return size; 
}

size_t _uart_getn(serial_dev_t self, uint8_t *data, size_t sz){
	size_t count = 0;
	while(sz--){
		int c = _uart_getc(self); 
		if(c == SERIAL_NO_DATA){
			return count; 
		}
		*data = c; 
		data++; 
		count++; 
	}
	return count; 
}

size_t _uart_waiting(serial_dev_t self){
	GET_DEV(self, dev);
	switch(dev->id){
		#define LABEL(id) case id: return uart##id##_waiting(); break; 
		#ifdef CONFIG_HAVE_UART0
		LABEL(0)
		#endif
		#ifdef CONFIG_HAVE_UART1
		LABEL(1)
		#endif
		#ifdef CONFIG_HAVE_UART2
		LABEL(2)
		#endif
		#ifdef CONFIG_HAVE_UART3
		LABEL(3)
		#endif
		#undef LABEL
	}
	return 0; 
}

int16_t _uart_begin(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	return 0; 
}

int16_t _uart_end(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	return 0; 
}

serial_dev_t uart_get_serial_interface(uint8_t dev){
	if(dev >= UART_COUNT) return 0;  // currently only one device TODO
	static struct serial_if _if;
	static struct serial_if *i = 0; 
	if(!i){
		_if = (struct serial_if) {
			.put = _uart_putc,
			.get = _uart_getc,
			.putn = _uart_putn,
			.getn = _uart_getn,
			.begin = _uart_begin,
			.end = _uart_end,
			.waiting = _uart_waiting
		}; 
		i = &_if; 
	}
	_uart[dev].serial = i;
	_uart[dev].id = dev; 
	return &_uart[dev].serial; 
}

void initproc uart_init(void){
	int c = 0; 
#ifdef CONFIG_HAVE_UART0
	uart0_init(UART_DEFAULT_BAUDRATE); 
	c++; 
#endif
#ifdef CONFIG_HAVE_UART1
	uart1_init(UART_DEFAULT_BAUDRATE); 
	c++; 
#endif
#ifdef CONFIG_HAVE_UART2
	uart2_init(UART_DEFAULT_BAUDRATE);  
	c++; 
#endif
#ifdef CONFIG_HAVE_UART3
	uart3_init(UART_DEFAULT_BAUDRATE); 
	c++; 
#endif
	sei(); 
	kdebug("UART: started %d uarts\n", c); 
}

/*
size_t uart0_putn(const char *buf, size_t size){
	return serial_putn(uart_get_serial_interface(0), (uint8_t*)buf, size); 
}*/
