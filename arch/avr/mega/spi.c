#include <arch/soc.h>
#include "spi.h"

int16_t spi_init_device(uint8_t dev){
	if(dev != 0) return -1; 
	spi0_init_default(); 
	return 0; 
}

uint16_t spi_transfer(uint8_t dev, uint8_t byte){
	if(dev != 0) return SERIAL_NO_DATA; 
	return spi0_transfer(byte); 
}
