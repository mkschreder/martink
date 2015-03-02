#pragma once 

#define AT24_PAGE_SIZE 32
#define AT24_ADDRESS_SIZE 2

struct at24_op {
	ssize_t size; // total length
	uint16_t addr; // eeprom address
	uint8_t buffer[AT24_PAGE_SIZE + AT24_ADDRESS_SIZE]; 
	ssize_t result; 
	int16_t error; 
	timestamp_t timeout; 
}; 

struct at24 {
	i2c_dev_t i2c;
	struct at24_op op; 
	struct libk_thread thread; 
	struct pt *user_thread; 
	struct block_device *dev; 
	uint8_t status; 
};

typedef enum {
	AT24_FLAG_BUSY = (1 << 0), 
	AT24_FLAG_READ = (1 << 1), 
	AT24_FLAG_WRITE = (1 << 2), 
	AT24_FLAG_LOCKED = (1 << 3)
} at24_flag_t; 

/// initializes a new eeprom structure
void at24_init(struct at24 *self, i2c_dev_t i2c); 
/// aquires the eeprom for this thread
uint8_t at24_open(struct at24 *self); 
/// releases the eeprom for others
int8_t 	at24_close(struct at24 *self); 
/// starts a write to the eeprom (max 1 page at a time)
ssize_t at24_writepage(struct at24 *self, uint16_t addr, const uint8_t *buf, ssize_t count); 
/// starts a read sequence from the eeprom (max 1 page at a time)
ssize_t at24_readpage(struct at24 *self, uint16_t addr, uint8_t *buf, ssize_t count); 
/// checks if eeprom transacton is in progress
uint8_t at24_get_status(struct at24 *self, at24_flag_t flags); 
/// get block device geometry for this device
int8_t at24_get_geometry(struct at24 *self, struct block_device_geometry *geom); 

block_dev_t at24_get_block_device_interface(struct at24 *self); 
