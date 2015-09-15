#pragma once

struct linux_spidev {
	int fd; 
	struct spi_device *api; 
}; 

void linux_spidev_init(struct linux_spidev *self); 
spi_dev_t linux_spidev_to_interface(struct linux_spidev *self); 
