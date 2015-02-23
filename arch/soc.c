/**
	This file is part of martink kernel library

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

#include "soc.h"
#include <stdarg.h>
#include <util/list.h>

/*
void soc_init(void){
	//clock_init(); 
	timestamp_init(); 
	time_init(); 
	//uart_init(); 
	gpio_init();
	spi_init(); 
	twi_init(); 
	pwm_init(); 
}
*/
uint16_t serial_printf(serial_dev_t port, const char *fmt, ...){
	char buffer[64]; 
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf(buffer, sizeof(buffer), fmt, vl); 
	va_end(vl);
	serial_putn(port, (uint8_t*)buffer, n); 
	return n; 
}


LIST_HEAD(_running); 
LIST_HEAD(_idle); 
static uint32_t _fps = 0; 
static uint32_t _bfps = 0; 

void libk_create_thread(struct libk_thread *self, char (*func)(struct pt *), const char *name){
	PT_INIT(&self->thread); 
	self->proc = func; 
	self->name = name; 
	self->time = 0; 
	list_add_tail(&self->list, &_running); 
}

void libk_delete_thread(struct libk_thread *self){
	PT_INIT(&self->thread); 
	self->proc = 0; 
	self->name = 0; 
	self->time = 0; 
	list_del_init(&self->list); 
}

void libk_schedule(void){
	struct list_head *ptr, *n; 
	static timestamp_t timeout = 0; 
	list_for_each_safe(ptr, n, &_running){
		struct libk_thread *thr = container_of(ptr, struct libk_thread, list); 
		timestamp_t time = timestamp_now(); 
		thr->proc(&thr->thread); 
		timestamp_t tnow = timestamp_now(); 
		if(tnow > time)
			thr->timecount += tnow - time;
		thr->framecount++; 
	}
	if(timestamp_expired(timeout)){
		list_for_each_safe(ptr, n, &_running){
			struct libk_thread *thr = container_of(ptr, struct libk_thread, list); 
			thr->time = (thr->time + timestamp_ticks_to_us(thr->timecount)) / 2; 
			thr->framecount = 0; 
			thr->timecount = 0; 
		}
		timeout = timestamp_from_now_us(1000000); 
	}
}

void libk_run(void){
	while(1){
		libk_schedule(); 
	}
}

uint32_t libk_get_fps(void){
	return _fps; 
}

void libk_print_info(void){
	struct list_head *ptr, *n; 
	printf("Running threads: \n"); 
	uint16_t count = 0; 
	long unsigned int ustotal = 0; 
	list_for_each_safe(ptr, n, &_running){
		struct libk_thread *thr = container_of(ptr, struct libk_thread, list); 
		long unsigned int time = thr->time; 
		printf("%16s: %4lu\n", thr->name, time / 10000); 
		count++; 
		ustotal += thr->time; 
	}
	printf("TOTAL: %d threads, %lu us\n", count, ustotal); 
	printf("FPS: %lu\n", libk_get_fps()); 
}

LIBK_THREAD(_fps_count){
	static timestamp_t time = 0; 
	_bfps++; 
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
		_fps = _bfps; 
		_bfps = 0; 
		time = timestamp_from_now_us(1000000); 
		PT_YIELD(pt); 
	}
	PT_END(pt); 
}

/*
LIBK_THREAD(_main_thread){
	static timestamp_t time; 
	
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
		printf("Main thread!\n"); 
		time = timestamp_from_now_us(1000000); 
	}
	PT_END(pt); 
}
*/
