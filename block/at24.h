#pragma once 

struct at24 {
	
	i2c_dev_t i2c;
};

void at24_init(struct at24 *self, i2c_dev_t i2c); 
size_t at24_write(struct at24 *self, uint16_t addr, const uint8_t *buf, size_t count); 
size_t at24_read(struct at24 *self, uint16_t addr, uint8_t *buf, size_t count); 
