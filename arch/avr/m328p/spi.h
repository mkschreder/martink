/**
	Fast macro based SPI interface for AVR Mega 328P

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

#ifndef _SPI_H_
#define _SPI_H_

#include <avr/io.h>

#include "autoconf.h"

#define SPI_CLOCK_DIV2 	(SPSR |= _BV(SPI2X))
#define SPI_CLOCK_DIV4	(0)
#define SPI_CLOCK_DIV8	(SPSR |= _BV(SPI2X), SPCR |= _BV(SPR0))
#define SPI_CLOCK_DIV16	(SPCR |= _BV(SPR0))
#define SPI_CLOCK_DIV32 (SPSR |= _BV(SPI2X), SPCR |= _BV(SPR1))
#define SPI_CLOCK_DIV64 (SPCR |= _BV(SPR1))
#define SPI_CLOCK_DIV128 (SPCR |= _BV(SPR0) | _BV(SPR1))

#define SPI_MODE0 (0)
#define SPI_MODE1 (_BV(CPHA))
#define SPI_MODE2 (_BV(CPOL))
#define SPI_MODE3 (_BV(CPOL)|_BV(CPHA))

#define GPIO_SPI0_MISO 	GPIO_MISO
#define GPIO_SPI0_MOSI	GPIO_MOSI
#define GPIO_SPI0_SCK		GPIO_SCK
#define GPIO_SPI0_SS		GPIO_SS

#define hwspi0_set_clock(spi_rate) ({\
	SPCR &= ~(_BV(SPR0) | _BV(SPR1));\
	SPSR &= ~(_BV(SPI2X));\
	spi_rate;\
})

#define hwspi0_set_mode(spi_mode) (SPCR = ((SPCR & ~(_BV(CPOL)|_BV(CPHA))) | spi_mode))
#define hwspi0_master() 					(SPCR |= _BV(MSTR))
#define hwspi0_slave() 						(SPCR &= ~_BV(MSTR))
#define hwspi0_order_lsb_first() 	(SPCR |= _BV(DORD))
#define hwspi0_order_msb_first() 	(SPCR &= ~_BV(DORD))
#define hwspi0_interrupt_enable() (SPCR |= _BV(SPIE))
#define hwspi0_interrupt_disable() (SPCR &= ~_BV(SPIE))
#define hwspi0_enable() 					(SPCR |= _BV(SPE))
#define hwspi0_disable() 					(SPCR &= ~_BV(SPE))
#define hwspi0_config_gpio()			(\
	gpio_set_function(GPIO_SPI0_MISO, GP_INPUT),\
	gpio_set_pullup(GPIO_SPI0_MISO, 1),\
	gpio_set_function(GPIO_SPI0_MOSI, GP_OUTPUT),\
	gpio_set_function(GPIO_SPI0_SS, GP_OUTPUT),\
	gpio_set_function(GPIO_SPI0_SCK, GP_OUTPUT)\
)
#define hwspi0_wait_for_transmit_complete() ({while((SPSR & (1<<SPIF)) == 0);})
#define hwspi0_error_collision() (SPSR & _BV(WCOL))

#define hwspi0_init() ({\
	hwspi0_config_gpio();\
	hwspi0_enable();\
	hwspi0_interrupt_disable(); \
	hwspi0_order_msb_first();\
	hwspi0_master();\
	hwspi0_set_clock(SPI_CLOCK_DIV128);\
	hwspi0_set_mode(SPI_MODE0);\
})

#define hwspi0_putc(ch) ({\
	SPDR = ch;\
	if(hwspi0_error_collision()) {\
		hwspi0_wait_for_transmit_complete();\
		SPDR = ch; \
	}\
})

#define hwspi0_getc(ch) (\
	hwspi0_wait_for_transmit_complete(),\
	SPDR \
)

#define hwspi0_transfer(ch) (\
	hwspi0_putc(ch),\
	hwspi0_getc()\
)

void PFDECL(CONFIG_SPI0_NAME, init, void);
uint8_t PFDECL(CONFIG_SPI0_NAME, writereadbyte, uint8_t data);

#endif
