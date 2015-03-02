/**
	A generic packet interface for the on board twi peripherals

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

#include <arch/soc.h>

#include "twi.h"

#define DEVICE_CAST(from, to) struct twi_device *to = container_of(from, struct twi_device, interface);  

struct twi_device {
	uint8_t id;
	struct i2c_interface *interface; 
};

static struct twi_device _twi[4] = {
	{.id = 0, .interface = 0},
	{.id = 1, .interface = 0},
	{.id = 2, .interface = 0},
	{.id = 3, .interface = 0}
}; 

static int16_t 			_twi_stop(i2c_dev_t self){
	DEVICE_CAST(self, dev);
	return i2cdev_stop(dev->id); 
}

static uint32_t	_twi_write(i2c_dev_t self, uint8_t adr, const uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	return i2cdev_write(dev->id, adr, data, max_sz); 
}

static uint32_t	_twi_read(i2c_dev_t self, uint8_t adr, uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	return i2cdev_read(dev->id, adr, data, max_sz); 
}
/*
static void			_twi_wait(i2c_dev_t self, uint8_t addr){
	DEVICE_CAST(self, dev);
	twi_wait(dev->id, addr); 
}
*/

static uint8_t			_twi_status(i2c_dev_t self, uint16_t status){
	DEVICE_CAST(self, dev);
	return i2cdev_status(dev->id, status); 
}

static uint8_t			_twi_open(i2c_dev_t self){
	DEVICE_CAST(self, dev);
	return i2cdev_open(dev->id); 
}

static void			_twi_close(i2c_dev_t self){
	DEVICE_CAST(self, dev);
	i2cdev_close(dev->id); 
}

i2c_dev_t twi_get_interface(uint8_t id){
	uint8_t count = sizeof(_twi) / sizeof(_twi[0]); 
	if(id >= count) return 0; 
	
	static struct i2c_interface _if;
	_if = (struct i2c_interface) {
		.write = 	_twi_write,
		.read = 	_twi_read,
		.stop = 		_twi_stop, 
		.status = _twi_status, 
		.open = _twi_open, 
		.close = _twi_close, 
	};
	_twi[id].interface = &_if; 
	return &_twi[id].interface; 
}


uint32_t i2c_write(i2c_dev_t dev, uint8_t address, const uint8_t *data, uint16_t max_sz){
	return (*dev)->write(dev, address, data, max_sz);
}

uint32_t	i2c_read(i2c_dev_t dev, uint8_t address, uint8_t *data, uint16_t max_sz){
	return (*dev)->read(dev, address, data, max_sz);
}

int16_t i2c_stop(i2c_dev_t dev){
	return (*dev)->stop(dev); 
}
/*
void i2c_wait(i2c_dev_t dev, uint8_t addr){
	(*dev)->wait(dev, addr); 
}
*/
uint8_t i2c_status(i2c_dev_t dev, uint16_t flags){
	return (*dev)->status(dev, flags); 
}

uint8_t i2c_open(i2c_dev_t dev){
	return (*dev)->open(dev); 
}

void i2c_close(i2c_dev_t dev){
	(*dev)->close(dev); 
}

/*
int8_t i2c_read_async(i2c_dev_t i2c, uint8_t addr, uint8_t reg, uint8_t *buff, uint8_t wr_bytes, uint8_t bytes){
	
}

int8_t i2c_read_async_sp(i2c_dev_t i2c, uint8_t addr, uint8_t reg, uint8_t *buff, uint8_t wr_bytes, uint8_t bytes){
	
}

int8_t i2c_write_async(i2c_dev_t i2c, uint8_t addr, uint8_t *buff, uint8_t bytes){
	
}*/
/*
PT_THREAD(i2c_read_reg_thread(i2c_dev_t i2c, struct pt *thr, uint8_t addr, uint8_t reg, uint8_t *buff, uint8_t bytes)){
	PT_BEGIN(thr); 
	
	if(bytes < 1) PT_EXIT(thr); 
	
	buff[0] = reg;
	
	PT_WAIT_UNTIL(thr, i2c_aquire(i2c)); 
	
	i2c_start_write(i2c, addr, buff, 1);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_start_read(i2c, addr, buff, bytes);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_stop(i2c); 
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_release(i2c); 
	
	PT_END(thr); 
}

PT_THREAD(i2c_write_read_thread(i2c_dev_t i2c, struct pt *thr, uint8_t addr, uint8_t *buff, uint8_t wr_bytes, uint16_t rd_bytes)){
	PT_BEGIN(thr); 
	
	PT_WAIT_UNTIL(thr, i2c_aquire(i2c)); 
	
	i2c_start_write(i2c, addr, buff, wr_bytes);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_start_read(i2c, addr, buff, rd_bytes);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_stop(i2c); 
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_release(i2c); 
	
	PT_END(thr); 
}

/// same as read_reg but with a stop (split transaction)
PT_THREAD(i2c_read_reg_thread_sp(i2c_dev_t i2c, struct pt *thr, uint8_t addr, uint8_t reg, uint8_t *buff, uint8_t bytes)){
	PT_BEGIN(thr); 
	
	if(bytes < 1) PT_EXIT(thr); 
	
	buff[0] = reg;
	
	PT_WAIT_UNTIL(thr, i2c_aquire(i2c)); 
	
	i2c_start_write(i2c, addr, buff, 1);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_stop(i2c); 
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_start_read(i2c, addr, buff, bytes);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_stop(i2c); 
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_release(i2c); 
	
	PT_END(thr); 
}

PT_THREAD(i2c_write_thread(i2c_dev_t i2c, struct pt *thr, uint8_t addr, uint8_t *buff, uint8_t bytes)){
	PT_BEGIN(thr); 
	
	PT_WAIT_UNTIL(thr, i2c_aquire(i2c)); 
	
	i2c_start_write(i2c, addr, buff, bytes);
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_stop(i2c); 
	PT_WAIT_WHILE(thr, i2c_busy(i2c)); 
	
	i2c_release(i2c); 
	
	PT_END(thr); 
}

*/
