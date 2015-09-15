#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <arch/soc.h>
#include <kernel/dev/spi.h>
#include "spidev.h"

static int _linux_spidev_open(spi_dev_t dev){
	struct linux_spidev *self = container_of(dev, struct linux_spidev, api); 
	self->fd = open("/dev/spidev0.0", O_RDWR); 
	if(self->fd <= 0) return -1; 
	return 0; 	
}

static int _linux_spidev_close(spi_dev_t dev){
	struct linux_spidev *self = container_of(dev, struct linux_spidev, api); 
	if(self->fd) close(self->fd); 
	return 0; 
}

static int _linux_spidev_transfer(spi_dev_t dev, const uint8_t *txbuf, uint8_t *rxbuf, size_t bufsize){
	struct linux_spidev *self = container_of(dev, struct linux_spidev, api); 
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)txbuf,
		.rx_buf = (unsigned long)rxbuf,
		.len = bufsize,
		.delay_usecs = 100,
		.speed_hz = 500000,
		.bits_per_word = 8,
	};	
	
	int ret = ioctl(self->fd, SPI_IOC_MESSAGE(1), &tr); 
	if(ret < 0) return -1; 
	return ret; 
}

void linux_spidev_init(struct linux_spidev *self){
	memset(self, 0, sizeof(struct linux_spidev)); 
}

spi_dev_t linux_spidev_to_interface(struct linux_spidev *self){
	static struct spi_device api = {
		.open = _linux_spidev_open, 
		.close = _linux_spidev_close, 
		.transfer = _linux_spidev_transfer
	}; 
	self->api = &api; 
	return &self->api; 
}
