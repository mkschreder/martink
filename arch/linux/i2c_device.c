#include <arch/soc.h>
#include "i2c_device.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>
#include <memory.h>

int linux_i2c_device_open(struct linux_i2c_device *self, int adapter){
	char filename[20];

	snprintf(filename, 19, "/dev/i2c-%d", adapter);
	self->fd = open(filename, O_RDWR);
	if (self->fd < 0) 
		return self->fd; 
	
	return 0; 
}

int linux_i2c_device_read(struct linux_i2c_device *self, 
	uint8_t addr, size_t length, uint8_t *data){
	if (ioctl(self->fd, I2C_SLAVE, addr) < 0)
		return -1; 
	return read(self->fd, data, length);
}

int linux_i2c_device_write(struct linux_i2c_device *self, 
	uint8_t addr, uint8_t reg, size_t length, uint8_t *data){
	if (ioctl(self->fd, I2C_SLAVE, addr) < 0)
		return -1; 
	char buffer[32]; 
	length = (length > sizeof(buffer) - 1 )?sizeof(buffer):length; 
	buffer[0] = reg; 
	memcpy(buffer + 1, data, length); 
	return write(self->fd, data, length);
}
