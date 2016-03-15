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

#include <kernel/mt.h>
#include <kernel/errno.h>
#include "i2c.h"

static LIST_HEAD(_i2c_adapters); 
static LIST_HEAD(_i2c_clients); 
static LIST_HEAD(_i2c_drivers); 

void i2c_register_adapter(struct i2c_adapter *adapter){
	INIT_LIST_HEAD(&adapter->list); 
	list_add_tail(&adapter->list, &_i2c_adapters); 
}

struct i2c_adapter *i2c_get_adapter(int number){
	return list_get_entry(&_i2c_adapters, number, struct i2c_adapter, list); 
}

void i2c_add_client_driver(struct i2c_driver *driver){
	INIT_LIST_HEAD(&driver->driver.list); 
	list_add_tail(&driver->driver.list, &_i2c_drivers); 
}

int i2c_add_device(struct i2c_adapter *adapter, uint8_t address, const char *driver){
	if(!adapter) return -EINVAL; 

	struct i2c_client *cl = kzmalloc(sizeof(struct i2c_client)); 
	cl->addr = address << 1; 
	cl->adapter = adapter; 
	INIT_LIST_HEAD(&cl->list); 

	// TODO: we add the client regardless of whether probe succeeds or fails because deallocating memory is bad here 
	list_add_tail(&cl->list, &_i2c_clients); 

	struct i2c_driver *_driver; 

	// find the client driver we will use for setting up this client
	list_for_each_entry(_driver, &_i2c_drivers, driver.list){
		if(strcmp(driver, _driver->driver.name) == 0 && _driver->probe && _driver->probe(cl) == 0){
			return 0; 
		}
	}
	
	return -EINVAL; 
}

int i2c_adapter_device_exists(struct i2c_adapter *adapter, uint8_t address){
	struct i2c_client cl; 
	memset(&cl, 0, sizeof(cl)); 
	cl.addr = address << 1; 
	cl.adapter = adapter; 
	char ch = 0; 
	if(i2c_client_read(&cl, &ch, 1) < 0) return 0; 
	return 1; 
}
