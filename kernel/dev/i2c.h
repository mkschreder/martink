#pragma once

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
