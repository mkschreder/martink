#pragma once 

#define AT24_PAGE_SIZE 32
#define AT24_ADDRESS_SIZE 2

#include <kernel/dev/block.h>
#include <kernel/thread.h>
#include <block/i2cblk.h>

#define AT24_PAGE_SIZE 32 

struct at24 {
	struct i2c_block_device i2cblk;
	block_dev_t i2cdev; 
	ssize_t size; 
	//struct block_transfer tr; 
	//struct pt *user_thread; 
	//uint16_t cur; 
	//uint8_t buffer[AT24_PAGE_SIZE]; 
	struct block_device_ops *dev; 
	//uint8_t status; 
};

typedef enum {
	AT24_FLAG_BUSY = (1 << 0), 
	AT24_FLAG_READ = (1 << 1), 
	AT24_FLAG_WRITE = (1 << 2), 
	AT24_FLAG_LOCKED = (1 << 3)
} at24_flag_t; 

/// initializes a new eeprom structure
void at24_init(struct at24 *self, block_dev_t i2c); 
/*
/// aquires the eeprom for this thread
uint8_t at24_open(struct at24 *self); 
/// releases the eeprom for others
int8_t 	at24_close(struct at24 *self); 
/// starts a write to the eeprom (max 1 page at a time)
ssize_t at24_write(struct at24 *self, const uint8_t *buf, ssize_t count); 
/// starts a read sequence from the eeprom (max 1 page at a time)
ssize_t at24_read(struct at24 *self, uint8_t *buf, ssize_t count); 
/// seeks to a position in the eeprom
ssize_t at24_seek(struct at24 *self, ssize_t pos, int from); 
/// checks if eeprom transacton is in progress
uint8_t at24_get_status(struct at24 *self, at24_flag_t flags); 
/// get block device geometry for this device
int8_t at24_get_geometry(struct at24 *self, struct block_device_geometry *geom); 
*/
block_dev_t at24_get_block_device_interface(struct at24 *self); 
