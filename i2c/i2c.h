/**
	Microcontroller I2C subsystem
	
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

#include <kernel/list.h>
#include <kernel/driver.h>

#define I2C_READ    1
#define I2C_WRITE   0

struct i2c_adapter; 

struct i2c_adapter_ops {
	int (*write)(struct i2c_adapter *self, uint8_t addr, const char *data, size_t size); 
	int (*read)(struct i2c_adapter *self, uint8_t addr, char *data, size_t size); 
	int (*transfer)(struct i2c_adapter *self, uint8_t addr, const char *wr_data, size_t wr_size, char *rd_data, size_t rd_size); 
}; 

struct i2c_adapter {
	//struct list_head list; 
	//struct i2c_driver *driver; 
	struct i2c_adapter_ops *ops; 
}; 

#define i2c_transfer(adapter, addr, wr, wr_size, rd, rd_size) (adapter)->ops->transfer(adapter, addr, wr, wr_size, rd, rd_size)
#define i2c_write(adapter, addr, wr, wr_size) (adapter)->ops->write(adapter, addr, wr, wr_size)
#define i2c_read(adapter, addr, wr, wr_size) (adapter)->ops->read(adapter, addr, wr, wr_size)

int i2c_device_exists(struct i2c_adapter *adapter, uint8_t address); 

struct i2c_adapter *atmega_i2c_get_adapter(int number); 
