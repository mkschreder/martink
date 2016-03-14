/*
	MCU Firmware Project

	Copyright (C) 2016 Martin K. Schröder <mkschreder.uk@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. (Please read LICENSE file on special
	permission to include this software in signed images). 

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	Git: https://github.com/mkschreder
*/

#pragma once

#include <kernel/types.h>
#include <kernel/errno.h>
#include <kernel/list.h>

struct serial_device; 
struct serial_device_ops {
	int (*write)(struct serial_device *dev, const char *data, size_t size); 
	int (*read)(struct serial_device *dev, char *data, size_t size); 
	void (*set_baud)(struct serial_device *dev, uint32_t baud);
}; 

struct serial_device {
	struct list_head list; 
	const char *name; 
	struct serial_device_ops *ops; 		
}; 

extern struct serial_device *_default_system_console; 
#define printk(...) serial_printf(_default_system_console, ##__VA_ARGS__)

void register_serial_device(struct serial_device *dev); 

static inline int serial_write(struct serial_device *dev, const char *data, unsigned long size){
	if(dev->ops->write) return dev->ops->write(dev, data, size); 
	return -EOPNOTSUPP; 
}

static inline int serial_read(struct serial_device *dev, char *data, unsigned long size){
	if(dev->ops->read) return dev->ops->read(dev, data, size); 
	return -EOPNOTSUPP; 
}

static inline void serial_set_baud(struct serial_device *dev, unsigned int baud){
	if(dev->ops->set_baud) dev->ops->set_baud(dev, baud); 
}

extern int serial_printf(struct serial_device *dev, const char *fmt, ...); 
