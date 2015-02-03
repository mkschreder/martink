#pragma once

void spi0_init_default(void); 
uint8_t spi0_transfer(uint8_t byte); 

#define hwspi0_init_default() spi0_init_default()
#define hwspi0_transfer(a) spi0_transfer(a)
