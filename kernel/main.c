/*
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

/**
 \mainpage LibK firmware development library

 <b>Introduction</b>

 LibK is a driver support library providing portable device driver
 support for embedded applications.

 It is distributed under the GNU General Public License - see the
 accompanying LICENSE.txt file for more details.

 It is designed to even be able to compile on a desktop. The future
 may very well involve using libk to use the same device drivers for
 both arduino and a linux based board that uses linux to access i2c
 ports and other ports. LibK can basically be compiled as a native
 library. All that is necessary to run it on any board is to
 implement the arch interface for that board. For a native version
 this layer would use linux system calls to access i2c or SPI ports
 instead of accessing hardware registers directly - but for the
 drivers it does not matter.

 <b>Download source code</b>

 You can get libk source code from the project github page:

 https://github.com/mkschreder

 
*/
#include "kernel.h"
#include "time.h"

#include <setjmp.h>
/*
static void do_initcalls (void) {
	printf("init start: %p, init end: %p\n", &__initcall_start, &__initcall_end); 
	for(initcall_t *call_p = &__initcall_start; call_p < &__initcall_end; call_p++){
		fprintf (stderr, "call_p: %p\n", call_p);
		(*call_p)();
		fprintf (stderr, "call_p: done\n");
	}
} 
*/
#if 0
int main(void){	
	//async_queue_init(&ASYNC_GLOBAL_QUEUE); 
	//do_initcalls(); 
	libk_loop(); 
}
#endif
void libk_register_device(struct libk_device *self, ASYNC_PTR(int, async_process_t, proc), const char *name){
	//struct libk_device *self = (struct libk_device*)container_of(__self, struct libk_device, process); 
	async_process_init(&self->process, ASYNC_NAME(int, async_process_t, proc), name); 
	libk_register_process(&self->process); 
}

void libk_loop() {
	while(1){
		//timestamp_t start = timestamp_now(); 
		//uint8_t pcount = 
		ASYNC_RUN_PARALLEL(&ASYNC_GLOBAL_QUEUE); 
		timestamp_t now = tsc_read(); 
		//DEBUG("loop: %lu\n", timestamp_ticks_to_us(now - start)); 
		if(now >= ASYNC_GLOBAL_QUEUE.sleep_until) {
			#if 0
			DEBUG("no sleep! %d\n", (int)(ASYNC_GLOBAL_QUEUE.sleep_until - now)); 
			#endif
		} else {
			timestamp_t us = timestamp_ticks_to_us(ASYNC_GLOBAL_QUEUE.sleep_until - now);
			udelay(us); 
			//DEBUG("pc: %d s: %lu u: %ld, T: %ld \n", pcount, us, ASYNC_GLOBAL_QUEUE.sleep_until, timestamp_now()); 
		}
	}
}

extern int __cxa_guard_acquire(__guard *g);
extern void __cxa_guard_release (__guard *g); 
extern void __cxa_guard_abort (__guard *g); 
extern void __cxa_pure_virtual(void); 

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);}
void __cxa_guard_release (__guard *g) {*(char *)g = (char)1;}
void __cxa_guard_abort (__guard *g) {*(char*)g = (char)0;}
void __cxa_pure_virtual(void) {}

