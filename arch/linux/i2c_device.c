#include <arch/soc.h>
#include "i2c_device.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>
#include <memory.h>

void linux_i2c_device_init(struct linux_i2c_device *self, uint8_t adapter_id){
	memset(self, 0, sizeof(struct linux_i2c_device)); 
	self->adapter = adapter_id; 
}

int _linux_i2c_device_open(struct linux_i2c_device *self){
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", self->adapter);
	self->fd = open(filename, O_RDWR);
	if (self->fd < 0) 
		return self->fd; 
	
	return 0; 
}

int _linux_i2c_device_close(struct linux_i2c_device *self){
	if(self->fd) close(self->fd); 
	self->fd = 0; 
	return 0; 
}

int _linux_i2c_device_read(struct linux_i2c_device *self, 
	uint8_t addr, size_t length, uint8_t *data){
	if (ioctl(self->fd, I2C_SLAVE, addr) < 0)
		return -1; 
	return read(self->fd, data, length);
}

int _linux_i2c_device_write(struct linux_i2c_device *self, 
	uint8_t addr, uint8_t reg, size_t length, uint8_t *data){
	if (ioctl(self->fd, I2C_SLAVE, addr) < 0)
		return -1; 
	char buffer[32]; 
	length = (length > sizeof(buffer) - 1 )?sizeof(buffer):length; 
	buffer[0] = reg; 
	memcpy(buffer + 1, data, length); 
	return write(self->fd, data, length);
}

i2c_dev_t linux_i2c_device_get_interface(struct linux_i2c_device *self){
	static struct i2c_device api = {
		.open = _linux_i2c_device_open, 
		.close = _linux_i2c_device_close, 
		.read = _linux_i2c_device_read, 
		.write = _linux_i2c_device_write
	}; 
	self->api = &api; 
	return &self->api; 
}
