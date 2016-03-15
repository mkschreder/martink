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
struct i2c_driver; 
struct i2c_client; 

struct i2c_adapter_ops {
	int (*write)(struct i2c_client *self, const char *data, size_t size); 
	int (*read)(struct i2c_client *self, char *data, size_t size); 
	int (*transfer)(struct i2c_client *self, const char *wr_data, size_t wr_size, char *rd_data, size_t rd_size); 
}; 

struct i2c_adapter {
	struct list_head list; 
	struct i2c_driver *driver; 
	struct i2c_adapter_ops *ops; 
}; 

struct i2c_client; 

struct i2c_client {
	struct list_head list; 
	struct i2c_adapter *adapter; 
	uint8_t addr; 
	void *data; 
}; 

struct i2c_driver {
	struct device_driver driver; 
	int (*probe)(struct i2c_client *client); 
}; 

void i2c_register_adapter(struct i2c_adapter *self); 
struct i2c_adapter *i2c_get_adapter(int number); 
void i2c_add_client_driver(struct i2c_driver *driver); 
int i2c_add_device(struct i2c_adapter *adapter, uint8_t address, const char *driver); 

#define i2c_client_set_data(cl, data) do { cl->data = data; } while(0)
#define i2c_client_get_data(cl) (cl->data)

#define i2c_client_transfer(client, wr, wr_size, rd, rd_size) (client)->adapter->ops->transfer(client, wr, wr_size, rd, rd_size)
#define i2c_client_write(client, wr, wr_size) (client)->adapter->ops->write(client, wr, wr_size)
#define i2c_client_read(client, wr, wr_size) (client)->adapter->ops->read(client, wr, wr_size)

int i2c_adapter_device_exists(struct i2c_adapter *adapter, uint8_t address); 
