#pragma once

#define I2CBLK_IADDR0 	0
#define I2CBLK_IADDR8		(1 << 0)
#define I2CBLK_IADDR16 	(2 << 0)
#define I2CBLK_IADDR32	(4 << 0)
#define I2CBLK_IADDR_BITS (7) // mask

#define I2CBLK_SEND_STOP_AFTER_ADDR (1 << 3)

#define I2C_BLOCK_BUFFER_SIZE 16
#define I2C_BLOCK_TOTAL_SIZE (I2C_BLOCK_BUFFER_SIZE + 4)

// IOCTL
#define I2CBLK_SET_AW 2

#include <kernel/dev/block.h>
#include <kernel/io_device.h>

struct i2c_block_device {
	io_dev_t i2c; // i2c block device for this device
	uint8_t i2c_addr; // address of the i2c device on i2c bus
	ssize_t offset;  // current position in transaction
	int32_t addr; // current position in the block device
	uint16_t block_size; // size of the maximum number of bytes that can be read/written continuously. 
	uint8_t buffer[I2C_BLOCK_BUFFER_SIZE]; 
	uint8_t flags; // block device flags
	timestamp_t time; 
	struct io_device io; // our base class
	async_mutex_t lock, buffer_lock; 
}; 

void i2cblk_init(struct i2c_block_device *self, io_dev_t i2c, uint8_t i2c_addr, uint16_t block_size, uint8_t flags); 
io_dev_t i2cblk_get_interface(struct i2c_block_device *self); 
