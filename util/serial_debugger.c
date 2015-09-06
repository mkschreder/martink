/**
	Serial debugger module

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

#include <kernel/util.h>
#include "serial_debugger.h"

#define DEVICE_CAST(from, to) struct serial_debugger *to = container_of(from, struct serial_debugger, _ex_serial);  

static struct serial_if sdbg;

uint16_t 	serial_debugger_getc(serial_dev_t self);
uint16_t 	serial_debugger_putc(serial_dev_t self, uint8_t ch);
size_t 		serial_debugger_putn(serial_dev_t self, const uint8_t *data, size_t max_sz); 
size_t 		serial_debugger_getn(serial_dev_t self, uint8_t *data, size_t max_sz);
int16_t 			serial_debugger_begin(serial_dev_t self);
int16_t 			serial_debugger_end(serial_dev_t self);
size_t 		serial_debugger_waiting(serial_dev_t self);

void serial_debugger_init(
	struct serial_debugger *dbg, 
	serial_dev_t device, 
	serial_dev_t console){
	dbg->device = device; 
	dbg->console = console; 
	dbg->buf_ptr = 0; 
	dbg->last_is_read = 2; 
	dbg->_ex_serial = 0; 
}

serial_dev_t serial_debugger_get_serial_interface(struct serial_debugger *self){
	sdbg = (struct serial_if){
		.get = serial_debugger_getc, 
		.put = serial_debugger_putc, 
		.getn = serial_debugger_getn, 
		.putn = serial_debugger_putn, 
		.waiting = serial_debugger_waiting, 
		.begin = serial_debugger_begin,
		.end = serial_debugger_end
	};
	self->_ex_serial = &sdbg;
	return &self->_ex_serial; 	
}

static void _do_flush(struct serial_debugger *dbg){
	if(dbg->last_is_read == 1){
		printf("sent: "); 
	} else {
		printf("got : "); 
	}
	
	for(int c = 0; c < dbg->buf_ptr; c++){
		printf("%02x ", dbg->buffer[c]); 
	}
	for(int c = 0; c < dbg->buf_ptr; c++){
		uint8_t i = dbg->buffer[c]; 
		printf("%c ", (i > 0x20 && i < 0x70)?i:'.'); 
	}
	printf("\n"); 
	dbg->buf_ptr = 0; 
}

static void _on_put(struct serial_debugger *dbg, uint16_t ch){
	dbg->buffer[dbg->buf_ptr++] = ch; 
	if(dbg->buf_ptr == 16 || (dbg->last_is_read == 1)) {
		_do_flush(dbg); 
		dbg->buf_ptr = 0; 
	}
	dbg->last_is_read = 0; 
}

static void _on_get(struct serial_debugger *dbg, uint16_t ch){
	if(ch != SERIAL_NO_DATA){
		dbg->buffer[dbg->buf_ptr++] = ch; 
	}
	if(dbg->buf_ptr == 16 || dbg->last_is_read == 0){
		_do_flush(dbg); 
	}
	dbg->last_is_read = 1; 
}

uint16_t serial_debugger_getc(serial_dev_t self){
	DEVICE_CAST(self, dbg); 
	//assert(dbg->device); 
	uint16_t d = serial_getc(dbg->device); 
	_on_get(dbg, d); 
	return d; 
}

uint16_t serial_debugger_putc(serial_dev_t self, uint8_t ch){
	DEVICE_CAST(self, dbg); 
	//assert(dbg->device); 
	uint16_t r = serial_putc(dbg->device, ch); 
	_on_put(dbg, ch); 
	return r; 
}

size_t serial_debugger_putn(serial_dev_t self, const uint8_t *data, size_t max_sz){
	DEVICE_CAST(self, dbg); 
	for(unsigned c = 0; c < max_sz; c++) _on_put(dbg, *data++); 
	//assert(dbg->device);
	return serial_putn(dbg->device, data, max_sz); 
}

size_t serial_debugger_getn(serial_dev_t self, uint8_t *data, size_t max_sz){
	DEVICE_CAST(self, dbg); 
	//assert(dbg->device);
	size_t ret = serial_getn(dbg->device, data, max_sz); 
	for(unsigned c = 0; c < max_sz; c++) _on_get(dbg, *data++); 
	return ret; 
}

int16_t serial_debugger_end(serial_dev_t self){
	DEVICE_CAST(self, dbg); 
	//assert(dbg->device);
	serial_end(dbg->device); 
	_do_flush(dbg); 
	return 0; 
}


int16_t serial_debugger_begin(serial_dev_t self){
	DEVICE_CAST(self, dbg); 
	//assert(dbg->device);
	serial_begin(dbg->device); 
	return 0; 
}

size_t serial_debugger_waiting(serial_dev_t self){
	DEVICE_CAST(self, dbg); 
	//assert(dbg->device);
	return serial_waiting(dbg->device); 
}
