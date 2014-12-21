#pragma once

void hwspi0_init_default(void); 

#define hwspi0_wait_for_transmit_complete() ({\
	while ((SPI0->SPI_SR & SPI_SR_TDRE) == 0);\
})

#define hwspi0_putc(ch) (\
	hwspi0_wait_for_transmit_complete(),\
	SPI0->SPI_TDR = ch\
)

#define hwspi0_getc(ch) (\
	hwspi0_wait_for_transmit_complete(),\
	SPI0->SPI_RDR & 0xff \
)

uint8_t hwspi0_transfer(uint8_t data); 
