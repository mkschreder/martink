#include <arch/soc.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <errno.h>
#include <memory.h>

#include "i2c_device.h"

void linux_i2c_device_init(struct linux_i2c_device *self, uint8_t adapter_id, uint8_t address){
	memset(self, 0, sizeof(struct linux_i2c_device)); 
	self->adapter = adapter_id; 
	self->addr = address; 
}

static int _linux_i2c_device_open(i2c_dev_t dev){
	struct linux_i2c_device *self = container_of(dev, struct linux_i2c_device, api); 
	char filename[20];
	
	if(self->fd != 0) return -1; 

	snprintf(filename, 19, "/dev/i2c-%d", self->adapter);
	self->fd = open(filename, O_RDWR);
	
	if (self->fd < 0) {
		DEBUG("i2c: failed to open %s\n", filename); 
		return self->fd; 
	}
	
	return 0; 
}

static int _linux_i2c_device_close(i2c_dev_t dev){
	struct linux_i2c_device *self = container_of(dev, struct linux_i2c_device, api); 
	//DEBUG("i2c: close\n"); 
	if(self->fd) close(self->fd); 
	self->fd = 0; 
	return 0; 
}

static int _linux_i2c_device_read(i2c_dev_t dev, 
		uint8_t *data, uint16_t length){
	struct linux_i2c_device *self = container_of(dev, struct linux_i2c_device, api); 
	if (ioctl(self->fd, I2C_SLAVE, self->addr) < 0)
		return -1; 
	return read(self->fd, data, length);
}

static int _linux_i2c_device_write(i2c_dev_t dev, 
		const uint8_t *data, uint16_t length){
	struct linux_i2c_device *self = container_of(dev, struct linux_i2c_device, api); 
	/*DEBUG("i2c: "); 
	for(uint16_t c = 0; c < length; c++){
		DEBUG("%x ", data[c]); 
	}
	DEBUG("\n");*/
	if (ioctl(self->fd, I2C_SLAVE, self->addr) < 0)
		return -1; 
	return write(self->fd, data, length);
}

static uint8_t _linux_i2c_device_status(i2c_dev_t dev){
	(void)dev; 
	return I2CDEV_READY; 
}

i2c_dev_t linux_i2c_device_get_interface(struct linux_i2c_device *self){
	static struct i2c_interface api = {
		.open = _linux_i2c_device_open, 
		.close = _linux_i2c_device_close, 
		.read = _linux_i2c_device_read, 
		.write = _linux_i2c_device_write,
		.status = _linux_i2c_device_status
	}; 
	self->api = &api; 
	return &self->api; 
}
