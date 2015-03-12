#include <arch/soc.h>
#include "block_device.h"

enum {
	BLKDEV_CLOSED, 
	BLKDEV_OPEN
}; 

static uint8_t _block_device_open(block_dev_t dev){
	struct block_device *self = container_of(dev, struct block_device, api); 
	
	switch(self->state){
		case BLKDEV_CLOSED: 
			self->user_thread = libk_current_thread(); 
			self->state = BLKDEV_OPEN; 
			return 1; 
		case BLKDEV_OPEN: 
			if(self->user_thread == libk_current_thread()) return 1; 
			break; 
	}
	return 0; 
}

static int8_t _block_device_close(block_dev_t dev){
	struct block_device *self = container_of(dev, struct block_device, api); 
	
	switch(self->state){
		case BLKDEV_CLOSED: 
			return 1; 
		case BLKDEV_OPEN: 
			if(self->user_thread != libk_current_thread()) return -EACCES; 
			self->state = BLKDEV_CLOSED; 
			return 1; 
	}
	return -EINVAL; 
}

void block_device_init(struct block_device *self){
	self->state = BLKDEV_CLOSED; 
	self->user_thread = 0; 
	
}

uint8_t block_device_can_access(struct block_device *self){
	if(self->user_thread != libk_current_thread()) return 0; 
	if(self->state != BLKDEV_OPEN) return 0; 
	return 1; 
}

block_dev_t block_device_get_interface(struct block_device *self){
	static struct block_device_ops _if;
	static struct block_device_ops *i = 0; 
	
	if(!i){
		_if = (struct block_device_ops) {
			.open = _block_device_open, 
			.close = _block_device_close, 
			//.write = _i2cblk_write,
			//.read = _i2cblk_read,
			//.ioctl = _i2cblk_ioctl, 
			//.seek = _i2cblk_seek, 
			//.get_geometry = _i2cblk_get_geometry,
			//.get_status = _i2cblk_get_status
		}; 
		i = &_if; 
	}
	self->api = i; 
	return &self->api; 
}

