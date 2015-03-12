
/*
 Base class for a block device. Basic open / close state management
 */
 
#pragma once

#include <kernel/dev/block.h>

struct block_device {
	struct pt *user_thread; 
	uint8_t state; 
	struct block_device_ops *api; 
}; 

void block_device_init(struct block_device *self); 
uint8_t block_device_can_access(struct block_device *self); 
block_dev_t block_device_get_interface(struct block_device *self); 
