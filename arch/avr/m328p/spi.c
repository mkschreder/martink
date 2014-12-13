/**
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#include <arch/soc.h>

#include "spi.h"

#include <avr/io.h>
#include <avr/interrupt.h>


/*
 * spi initialize
 */
void PFDECL(CONFIG_SPI0_NAME, init, void) {
	hwspi0_init(); 
	/*
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
   */
}

/*
 * spi write one byte and read it back
 */
uint8_t PFDECL(CONFIG_SPI0_NAME, writereadbyte, uint8_t data) {
	return hwspi0_transfer(data); 
}


