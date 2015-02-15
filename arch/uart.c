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

//#define UART_DEFAULT_BAUDRATE 38400

struct uart_device {
	uint8_t id; 
	struct serial_if *serial; 
}; 

struct uart_device _uart[UART_COUNT]; 

#define GET_DEV(s, dev) \
	struct uart_device *dev = container_of((s), struct uart_device, serial)
	
static uint16_t _uart_putc(serial_dev_t self, uint8_t ch){
	GET_DEV(self, dev);
	uart_putc(dev->id, ch); 
	return SERIAL_NO_DATA; 
}

static uint16_t _uart_getc(serial_dev_t self) {
	GET_DEV(self, dev);
	return uart_getc(dev->id); 
}

static size_t _uart_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_uart_putc(self, *data++); 
	}
	return size; 
}

static size_t _uart_getn(serial_dev_t self, uint8_t *data, size_t sz){
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

static size_t _uart_waiting(serial_dev_t self){
	GET_DEV(self, dev);
	return uart_waiting(dev->id); 
}

static int16_t _uart_begin(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

static int16_t _uart_end(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

serial_dev_t uart_get_serial_interface(uint8_t dev_id){
	uint8_t count = sizeof(_uart) / sizeof(_uart[0]); 
	if(dev_id >= count) return 0; 
	
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
	_uart[dev_id].serial = i;
	_uart[dev_id].id = dev_id; 
	return &_uart[dev_id].serial; 
}

/*
size_t uart0_putn(const char *buf, size_t size){
	return serial_putn(uart_get_serial_interface(0), (uint8_t*)buf, size); 
}*/
