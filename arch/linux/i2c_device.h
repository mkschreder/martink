#pragma once

struct linux_i2c_device {
	int fd; 
	int addr; 
	i2c_dev_t vtable; 
}; 

int linux_i2c_device_open(struct linux_i2c_device *self, int adapter); 
int linux_i2c_device_read(struct linux_i2c_device *self, 
	uint8_t addr, size_t length, uint8_t *data);
int linux_i2c_device_write(struct linux_i2c_device *self, 
	uint8_t addr, uint8_t reg, size_t length, uint8_t *data); 
struct i2c_dev_t linux_i2c_device_interface(struct linux_i2c_device *self); 
