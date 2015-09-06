#pragma once

// ioctl
#define I2C_SEND_STOP 1000

typedef enum {
	I2CDEV_BUSY = (1 << 0), 
	I2CDEV_LOCKED = (1 << 1), 
	I2CDEV_READ = (1 << 2), 
	I2CDEV_WRITE = (1 << 3), 
	I2CDEV_READY = (1 << 4), 
	I2CDEV_START = (1 << 5), 
	I2CDEV_STOP = (1 << 6)
	//I2CDEV_TX_DONE = (1 << 5), 
	//I2CDEV_RX_NOT_EMPTY = (1 << 6)
} i2cdev_status_t; 

// combines i2c device address and block address into a block device address. 
#define I2C_REG_ADDRESS(dev_addr, reg_addr) ((((uint32_t)reg_addr << 8) | (uint8_t)dev_addr) << 8)

struct i2c_interface;
typedef struct i2c_interface **i2c_dev_t;


/**
 * I2C device interface used for reading and writing i2c devices.
 */
struct i2c_interface {
	uint32_t			(*write)(i2c_dev_t self,
		uint8_t address, const uint8_t *data, uint16_t max_sz);

	uint32_t			(*read)(i2c_dev_t self,
		uint8_t address, uint8_t *data, uint16_t max_sz);

	/// returns -1 on fail and 1 on success
	int16_t 			(*stop)(i2c_dev_t self); 
	
	//void	(*wait)(i2c_dev_t self, uint8_t addr); 
	
	uint8_t (*status)(i2c_dev_t self, uint16_t flags); 
	uint8_t (*open)(i2c_dev_t self); 
	void 		(*close)(i2c_dev_t self); 
};

uint32_t i2c_write(i2c_dev_t dev,
	uint8_t address, const uint8_t *data, uint16_t max_sz);

uint32_t	i2c_read(i2c_dev_t dev,
	uint8_t address, uint8_t *data, uint16_t max_sz);

int16_t i2c_stop(i2c_dev_t dev);

//void i2c_wait(i2c_dev_t dev, uint8_t addr); 

uint8_t i2c_status(i2c_dev_t dev, uint16_t status); 

uint8_t i2c_open(i2c_dev_t dev); 

void i2c_close(i2c_dev_t dev); 
