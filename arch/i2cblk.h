#pragma once

#define I2CBLK_IADDR0 	0
#define I2CBLK_IADDR8		(1 << 0)
#define I2CBLK_IADDR16 	(2 << 0)
#define I2CBLK_IADDR32	(4 << 0)
#define I2CBLK_IADDR_BITS (7) // mask

//#define I2CBLK_SEND_STOP_AFTER_ADDR (1 << 2)

#define I2C_BLOCK_BUFFER_SIZE (16 + 4)

// IOCTL
#define I2CBLK_SET_AW 2

#include <kernel/dev/block.h>
#include <kernel/transfer.h>
#include <block/block_device.h>

struct i2c_block_device {
	block_dev_t i2c; // i2c block device for this device
	uint8_t i2c_addr; // address of the i2c device on i2c bus
	struct block_transfer tr; 
	ssize_t cur;  // current file position
	uint8_t buffer[I2C_BLOCK_BUFFER_SIZE]; 
	uint8_t state; 
	uint8_t flags; // block device flags
	struct block_device base; 
	struct block_device_ops *api; 
	//struct libk_thread thread; 
	//struct pt *user_thread; 
}; 

void i2cblk_init(struct i2c_block_device *self, block_dev_t i2c, uint8_t i2c_addr); 
block_dev_t i2cblk_get_interface(struct i2c_block_device *self); 
