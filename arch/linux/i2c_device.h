#pragma once

#include <kernel/dev/i2c.h>

struct linux_i2c_device {
	int fd; 
	int addr;
	int adapter;
	struct i2c_interface *api; 
}; 

void linux_i2c_device_init(struct linux_i2c_device *self, uint8_t adapter, uint8_t address); 
int linux_i2c_device_open(struct linux_i2c_device *self, int adapter); 
int linux_i2c_device_close(struct linux_i2c_device *self); 
int linux_i2c_device_read(struct linux_i2c_device *self, 
	uint8_t addr, size_t length, uint8_t *data);
int linux_i2c_device_write(struct linux_i2c_device *self, 
	uint8_t addr, uint8_t reg, size_t length, uint8_t *data); 
i2c_dev_t linux_i2c_device_get_interface(struct linux_i2c_device *self); 
