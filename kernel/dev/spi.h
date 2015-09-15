#pragma once 

struct spi_device; 
typedef struct spi_device **spi_dev_t; 

struct spi_device {
	int (*open)(spi_dev_t dev); 
	int (*close)(spi_dev_t dev); 
	int (*transfer)(spi_dev_t dev, const uint8_t *tx_buf, uint8_t *rx_buf, size_t size); 
}; 

#define spi_open(dev) (*dev)->open(dev)
#define spi_close(dev) (*dev)->close(dev)
#define spi_transfer(dev, rx, tx, sz) (*dev)->transfer(dev, rx, tx, sz)
