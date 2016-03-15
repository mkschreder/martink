/**
	Micro Block Device System 
	
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

#include <kernel/types.h>
#include <kernel/list.h>

struct block_device; 
struct block_device_ops {
	int (*write)(struct block_device *dev, size_t offset, const char *data, size_t size); 
	int (*read)(struct block_device *dev, size_t offset, char *data, size_t size); 
}; 

struct block_device {
	struct list_head list; 
	struct block_device_ops *ops; 
}; 

void block_register_device(struct block_device *dev); 
struct block_device *block_get_device(int number); 

#define block_write(dev, offset, data, size) dev->ops->write(dev, offset, data, size)
#define block_read(dev, offset, data, size) dev->ops->read(dev, offset, data, size)
