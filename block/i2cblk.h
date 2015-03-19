#pragma once

#define I2CBLK_IADDR0 	0
#define I2CBLK_IADDR8		(1 << 0)
#define I2CBLK_IADDR16 	(2 << 0)
#define I2CBLK_IADDR32	(4 << 0)
#define I2CBLK_IADDR_BITS (7) // mask

#define I2CBLK_SEND_STOP_AFTER_ADDR (1 << 3)

#define I2C_BLOCK_BUFFER_SIZE (16 + 4)

// IOCTL
#define I2CBLK_SET_AW 2

#include <kernel/dev/block.h>
#include <kernel/io_device.h>

struct i2c_block_device {
	io_dev_t i2c; // i2c block device for this device
	uint8_t i2c_addr; // address of the i2c device on i2c bus
	ssize_t cur;  // current file position
	uint8_t buffer[I2C_BLOCK_BUFFER_SIZE]; 
	uint8_t flags; // block device flags
	struct io_device io; // our base class
}; 

void i2cblk_init(struct i2c_block_device *self, io_dev_t i2c, uint8_t i2c_addr); 
io_dev_t i2cblk_get_interface(struct i2c_block_device *self); 
