#pragma once

#ifndef CONFIG_SPI0_NAME
#define CONFIG_SPI0_NAME spi0
#endif

void PFDECL(CONFIG_SPI0_NAME, init, void);
uint8_t PFDECL(CONFIG_SPI0_NAME, writereadbyte, uint8_t b); 
