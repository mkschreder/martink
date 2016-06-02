/**
	Simple AT24 EEPROM driver
	
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


#include <kernel/kernel.h>
#include <arch/soc.h>
#include <i2c/i2c.h>
#include <serial/serial.h>
#include "memory.h"

//#define AT24_PAGE_SIZE 32 

struct at24 {
	struct memory_device device; 
	struct i2c_adapter *adapter; 
	uint8_t addr; 
};

static int at24_read(struct memory_device *dev, size_t offset, char *data, size_t size){
	if(!dev) return -EINVAL; 
	struct at24 *self = container_of(dev, struct at24, device); 
	char outbuf[2] = { offset >> 8, offset & 0xff }; 
	int ret = i2c_transfer(self->adapter, self->addr, outbuf, sizeof(outbuf), data, size); 
	if(ret < 0) return -EFAULT; 
	return ret; 
}

static int at24_write(struct memory_device *dev, size_t offset, const char *data, size_t _size){
	if(!dev) return -EINVAL; 
	struct at24 *self = container_of(dev, struct at24, device); 
	size_t size = _size; 
	// write one byte at a time (guaranteed support on all devices)  
	while(size--){
		char outbuf[3] = { offset >> 8, offset & 0xff, *data }; 
		if(i2c_write(self->adapter, self->addr, outbuf, sizeof(outbuf)) < 0){
			return -EFAULT; 
		}
		// twr is at least 10ms
		msleep(10); 
		offset++; data++; 
	}
	return _size - size; 
}

static struct memory_device_ops at24_ops = {
	.read = at24_read, 
	.write = at24_write
}; 

struct memory_device* at24_device_new(struct i2c_adapter *adapter, uint8_t addr) {
	if(!i2c_device_exists(adapter, addr)) return NULL; 
	struct at24 *self = kzmalloc(sizeof(struct at24)); 
	if(!self) return 0; 
	self->addr = addr; 
	self->device.ops = &at24_ops; 
	self->adapter = adapter; 
	return &self->device; 
}

