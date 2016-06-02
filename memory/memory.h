/**
	External Memory Device System 
	
	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

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
	Github: https://github.com/mkschreder
*/

#pragma once

#include <kernel/device.h>
#include <kernel/driver.h>
#include <kernel/types.h>
#include <kernel/list.h>
#include <i2c/i2c.h>

struct memory_device; 
struct memory_device_ops {
	int (*write)(struct memory_device *dev, size_t offset, const char *data, size_t size); 
	int (*read)(struct memory_device *dev, size_t offset, char *data, size_t size); 
}; 

struct memory_device {
	//struct device device; 
	struct memory_device_ops *ops; 
}; 
/*
void memory_register_device(struct memory_device *dev); 
struct memory_device *memory_get_device(const char *path); 
*/
#define memory_device_write(dev, offset, data, size) dev->ops->write(dev, offset, data, size)
#define memory_device_read(dev, offset, data, size) dev->ops->read(dev, offset, data, size)

struct memory_device* at24_device_new(struct i2c_adapter *adapter, uint8_t addr); 
struct memory_device *atmega_eeprom_get(void); 
