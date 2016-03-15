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
#include <block/block.h>

//#define AT24_PAGE_SIZE 32 

struct at24 {
	struct i2c_client *client; 
	struct block_device device; 
};

static int at24_read(struct block_device *dev, size_t offset, char *data, size_t size){
	if(!dev) return -EINVAL; 
	struct at24 *self = container_of(dev, struct at24, device); 
	char outbuf[2] = { offset >> 8, offset & 0xff }; 
	int ret = i2c_client_transfer(self->client, outbuf, sizeof(outbuf), data, size); 
	if(ret < 0) return -EFAULT; 
	return ret; 
}

static int at24_write(struct block_device *dev, size_t offset, const char *data, size_t _size){
	if(!dev) return -EINVAL; 
	struct at24 *self = container_of(dev, struct at24, device); 
	size_t size = _size; 
	// write one byte at a time (guaranteed support on all devices)  
	while(size--){
		char outbuf[3] = { offset >> 8, offset & 0xff, *data }; 
		if(i2c_client_write(self->client, outbuf, sizeof(outbuf)) < 0){
			return -EFAULT; 
		}
		// twr is at least 10ms
		msleep(10); 
		offset++; data++; 
	}
	return _size - size; 
}

static struct block_device_ops at24_ops = {
	.read = at24_read, 
	.write = at24_write
}; 

static int at24_probe(struct i2c_client *client) {
	struct at24 *data = kzmalloc(sizeof(struct at24)); 
	if(!data) return -ENOMEM; 
	data->client = client; 
	data->device.ops = &at24_ops; 
	block_register_device(&data->device); 
	i2c_client_set_data(client, data); 
	return 0; 
}

static struct i2c_driver at24_driver = {
	.driver = {
		.name = "at24"
	},
	.probe = at24_probe, 
	//.remove = at24_remove
}; 

static void __init at24_init(void){
	i2c_add_client_driver(&at24_driver); 	
}

