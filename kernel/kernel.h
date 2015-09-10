/**
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

#pragma once 

#ifdef __cplusplus
extern "C" {
#endif 

#include "config.h"

#include <inttypes.h>
#include <stddef.h>
//#include <sys/types.h>

#include <string.h>

#include <arch/soc.h>
#include "util.h"
#include "list.h"

#include "dev/block.h"
#include "dev/framebuffer.h"
#include "dev/i2c.h"
#include "dev/parallel.h"
#include "dev/serial.h"
#include "dev/tty.h"

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
__extension__ typedef int __guard __attribute__((mode (__DI__)));

extern "C" int __cxa_guard_acquire(__guard *);
extern "C" void __cxa_guard_release (__guard *);
extern "C" void __cxa_guard_abort (__guard *);
extern "C" void __cxa_pure_virtual(void);
#else 
	typedef int __guard; 
#endif


struct libk_device {
	timestamp_t timeout; 
	struct async_process process; 
}; 

#define DEVICE_CLOCK_PTR(name) ASYNC_NAME(int, async_process_t, name)
#define DEVICE_CLOCK_GET_DEVICE() container_of(__self, struct libk_device, process)
#define DEVICE_CLOCK(name) ASYNC_PROCESS(name)
#define DEVICE_DELAY(delay) AWAIT_DELAY(DEVICE_CLOCK_GET_DEVICE()->timeout, delay); 

/*
typedef int (*initcall_t)(void);
extern initcall_t __initcall_start, __initcall_end;

#define __initcall(fn) \
static initcall_t __initcall_##fn __init_call = fn
#define __init_call     __attribute__ ((unused,__section__ ("function_ptrs")))
#define module_init(x)  __initcall(x);

#define __init __attribute__ ((__section__ (".text")))
*/

#define __init __attribute__ ((constructor))

#define MODULE_AUTHOR(name)
#define MODULE_NAME(name)

#define libk_init_process(async_proc_struct_ptr, method) ASYNC_PROCESS_INIT(async_proc_struct_ptr, method)
#define libk_register_process(async_proc_struct_ptr) ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, async_proc_struct_ptr)
#define libk_loop() while(ASYNC_RUN_PARALLEL(&ASYNC_GLOBAL_QUEUE)) { NATIVE_USLEEP(10000); }
void libk_register_device(struct libk_device *self, ASYNC_PTR(int, async_process_t, proc), const char *name);
//#include "wiinunchuck.h"
