#pragma once 


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

int8_t 	i2cdev_init(uint8_t dev_id); 
void 		i2cdev_deinit(uint8_t dev_id); 
uint8_t i2cdev_open(uint8_t dev_id); 
void	 	i2cdev_close(uint8_t dev_id); 
int16_t 	i2cdev_write(uint8_t dev_id, uint8_t addr, const uint8_t *data, uint8_t sz); 
int16_t 	i2cdev_read(uint8_t dev_id, uint8_t addr, uint8_t *data, uint8_t sz); 
int8_t 	i2cdev_stop(uint8_t dev_id); 
uint8_t i2cdev_status(uint8_t dev_id, i2cdev_status_t flags); 
//void twi_wait(uint8_t dev_id, uint8_t addr); 
