/*
	MCU Firmware Project

	Copyright (C) 2016 Martin K. Schröder <mkschreder.uk@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	Git: https://github.com/mkschreder
*/

#include <arch/soc.h>
#include <stdio.h>
#include <serial/serial.h>
#include <kernel/types.h>
#include <kernel/mt.h>

static void _serial_fd_putc(char c, FILE *stream){
	struct serial_device *dev = fdev_get_udata(stream); 
	char ch = c; 
	serial_write(dev, (char*)&ch, 1); 
}

static int _serial_fd_getc(FILE *stream){
	struct serial_device *dev = fdev_get_udata(stream); 
	char ch; 
	serial_read(dev, &ch, 1); 
	return ch; 
}

// lock used for synchronizing printk calls so that we do not print garbage to the terminal
static mutex_t _printk_lock; 

#include <string.h>
#include <arch/soc.h>

int serial_printf(struct serial_device *dev, const char *fmt, ...){
	FILE fd; 

	if(!dev || !thread_self()) return -EFAULT; 

	mutex_lock(&_printk_lock); 
	fdev_setup_stream(&fd, _serial_fd_putc, _serial_fd_getc, _FDEV_SETUP_RW); 
	fdev_set_udata(&fd, dev); 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vfprintf(&fd, fmt, vl); 
	va_end(vl);
	mutex_unlock(&_printk_lock); 
	return n; 
}

static void __init _init_serial(void){
	mutex_init(&_printk_lock); 
}
