#pragma once

int16_t spi_init_device(uint8_t dev); 
uint8_t spi_transfer(uint8_t dev, uint8_t byte); 

/*
void spi1_init_default(void); 
uint8_t spi1_transfer(uint8_t byte); 
*/
