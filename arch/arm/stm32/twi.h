#pragma once 

#include <i2c/i2c.h>

#if 0 
int8_t 	i2cdev_init(uint8_t dev_id); 
void 		i2cdev_deinit(uint8_t dev_id); 
io_dev_t i2cdev_get_interface(uint8_t dev_id); 
#endif

/*
uint8_t i2cdev_open(uint8_t dev_id); 
void	 	i2cdev_close(uint8_t dev_id); 
int16_t 	i2cdev_write(uint8_t dev_id, uint8_t addr, const uint8_t *data, uint8_t sz); 
int16_t 	i2cdev_read(uint8_t dev_id, uint8_t addr, uint8_t *data, uint8_t sz); 
int8_t 	i2cdev_stop(uint8_t dev_id); 
uint8_t i2cdev_status(uint8_t dev_id, i2cdev_status_t flags); */
//void twi_wait(uint8_t dev_id, uint8_t addr); 
