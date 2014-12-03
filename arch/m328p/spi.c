/*
spi lib 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/

#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>


/*
 * spi initialize
 */
void spi_init(void) {
    SPI_DDR &= ~((1<<SPI_MISO)); //input
    SPI_DDR |= ((1<<SPI_MOSI) | (1<<SPI_SS) | (1<<SPI_SCK)); //output

		// pullup! 
		SPI_PORT |= (1<<SPI_MISO);
		
    SPCR = ((1<<SPE)|               // SPI Enable
            (0<<SPIE)|              // SPI Interupt Enable
            (0<<DORD)|              // Data Order (0:MSB first / 1:LSB first)
            (1<<MSTR)|              // Master/Slave select
            (1<<SPR1)|(1<<SPR0)|    // SPI Clock Rate
            (0<<CPOL)|              // Clock Polarity (0:SCK low / 1:SCK hi when idle)
            (0<<CPHA));             // Clock Phase (0:leading / 1:trailing edge sampling)

    SPSR = (0<<SPI2X); // Double SPI Speed Bit
}

/*
 * spi write one byte and read it back
 */
uint8_t spi_writereadbyte(uint8_t data) {
    SPDR = data;
    while((SPSR & (1<<SPIF)) == 0);
    return SPDR;
}


