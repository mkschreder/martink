#include <arch/soc.h>

#include "spi.h"

#define PIO_MOSI 	PIO_PA26A_SPI0_MOSI
#define PIO_MISO 	PIO_PA25A_SPI0_MISO
#define PIO_SCK 	PIO_PA27A_SPI0_SPCK

#define SPI_CLOCK_DIV2	 11
#define SPI_CLOCK_DIV4	 21
#define SPI_CLOCK_DIV8	 42
#define SPI_CLOCK_DIV16	 84
#define SPI_CLOCK_DIV32	 168
#define SPI_CLOCK_DIV64	 255
#define SPI_CLOCK_DIV128 255

#define SPI_MODE0 0x02
#define SPI_MODE1 0x00
#define SPI_MODE2 0x03
#define SPI_MODE3 0x01

void hwspi0_init_default(void){
	pmc_enable_periph_clk(ID_SPI0);
	pmc_enable_periph_clk(ID_PIOA);
	
	PIO_Configure(PIOA, PIO_PERIPH_A, PIO_MOSI, PIO_DEFAULT); 
	PIO_Configure(PIOA, PIO_PERIPH_A, PIO_MISO, PIO_DEFAULT); 
	PIO_Configure(PIOA, PIO_PERIPH_A, PIO_SCK, 	PIO_DEFAULT); 
	
	SPI_Configure(SPI0, ID_SPI0, SPI_MR_MSTR | SPI_MR_PS | SPI_MR_MODFDIS);
	SPI_Enable(SPI0);
	
	SPI_ConfigureNPCS(SPI0, 0, SPI_CSR_CSAAT | SPI_MODE0 | SPI_CSR_SCBR(SPI_CLOCK_DIV16) | SPI_CSR_DLYBCT(1));
	SPI_ConfigureNPCS(SPI0, 1, SPI_CSR_CSAAT | SPI_MODE0 | SPI_CSR_SCBR(SPI_CLOCK_DIV16) | SPI_CSR_DLYBCT(1));
	SPI_ConfigureNPCS(SPI0, 2, SPI_CSR_CSAAT | SPI_MODE0 | SPI_CSR_SCBR(SPI_CLOCK_DIV16) | SPI_CSR_DLYBCT(1));
	SPI_ConfigureNPCS(SPI0, 3, SPI_CSR_CSAAT | SPI_MODE0 | SPI_CSR_SCBR(SPI_CLOCK_DIV16) | SPI_CSR_DLYBCT(1));
	//SPI_ConfigureNPCS(SPI0, 0, SPI_BITS_8 | SCBR);
}


uint8_t hwspi0_transfer(uint8_t data){
	uint32_t ch = 0;
	uint32_t d = data | SPI_PCS(0);
	/*if (_mode == SPI_LAST)
		d |= SPI_TDR_LASTXFER;*/

	while ((SPI0->SPI_SR & SPI_SR_TDRE) == 0);
	SPI0->SPI_TDR = d;
	while ((SPI0->SPI_SR & SPI_SR_RDRF) == 0);
	d = SPI0->SPI_RDR;
	
	return d & 0xff;
}
