#include <arch/soc.h>
#include "block_device.h"

enum {
	BLKDEV_CLOSED, 
	BLKDEV_OPEN
}; 

uint8_t block_device_open(struct block_device *self){
	switch(self->state){
		case BLKDEV_CLOSED: 
			self->user_thread = libk_current_thread(); 
			self->state = BLKDEV_OPEN; 
			return 1; 
		case BLKDEV_OPEN: 
			//if(self->user_thread == libk_current_thread()) return 1; 
			break; 
	}
	return 0; 
}

int8_t block_device_close(struct block_device *self){
	switch(self->state){
		case BLKDEV_CLOSED: 
			return 1; 
		default: 
			if(self->user_thread != libk_current_thread()) return -EACCES; 
			self->state = BLKDEV_CLOSED; 
			self->user_thread = 0; 
			return 1; 
	}
	return -EINVAL; 
}

static uint8_t _block_device_open(block_dev_t dev){
	struct block_device *self = container_of(dev, struct block_device, api); 
	return block_device_open(self); 
}

static int8_t _block_device_close(block_dev_t dev){
	struct block_device *self = container_of(dev, struct block_device, api); 
	return block_device_close(self); 
}

void block_device_init(struct block_device *self){
	self->state = BLKDEV_CLOSED; 
	self->user_thread = 0; 
	
}

uint8_t block_device_can_access(struct block_device *self){
	if(self->state != BLKDEV_OPEN) return 0; 
	if(self->user_thread == libk_current_thread()) return 1; 
	//printf("ACCESS: %x %x\n", self->user_thread, libk_current_thread()); 
	return 0; 
}


static ssize_t _block_device_seek(block_dev_t dev, ssize_t pos, int whence){
	(void)dev; (void)pos; (void)whence; 
	return -EINVAL; 
}

static ssize_t _block_device_write(block_dev_t dev, const uint8_t *data, ssize_t count){
	(void)dev; (void)data; (void)count; 
	return -EINVAL; 
}

static ssize_t _block_device_read(block_dev_t dev, uint8_t *data, ssize_t count){
	(void)dev; (void)data; (void)count; 
	return -EINVAL; 
}
static int16_t _block_device_ioctl(block_dev_t dev, ioctl_req_t req, ...){
	(void)dev; 
	(void)req; 
	return -EINVAL; 
}

block_dev_t block_device_get_interface(struct block_device *self){
	static struct block_device_ops _if;
	static struct block_device_ops *i = 0; 
	
	if(!i){
		_if = (struct block_device_ops) {
			.open = _block_device_open, 
			.close = _block_device_close, 
			.write = _block_device_write,
			.read = _block_device_read,
			.ioctl = _block_device_ioctl, 
			.seek = _block_device_seek, 
			//.get_geometry = _i2cblk_get_geometry,
			//.get_status = _i2cblk_get_status
		}; 
		i = &_if; 
	}
	self->api = i; 
	return &self->api; 
}

