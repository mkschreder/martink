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
#include <arch/gpio.h>

#include "gpio.h"

#include "config.h"

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

static inline void spi0_set_clock(uint16_t spi_rate) {
	SPCR &= ~(_BV(SPR0) | _BV(SPR1));
	SPSR &= ~(_BV(SPI2X));
	SPCR |= spi_rate;
}

#define spi0_set_mode(spi_mode) (SPCR = ((SPCR & ~(_BV(CPOL)|_BV(CPHA))) | spi_mode))
#define spi0_master() 					(SPCR |= _BV(MSTR))
#define spi0_slave() 						(SPCR &= ~_BV(MSTR))
#define spi0_order_lsb_first() 	(SPCR |= _BV(DORD))
#define spi0_order_msb_first() 	(SPCR &= ~_BV(DORD))
#define spi0_interrupt_enable() (SPCR |= _BV(SPIE))
#define spi0_interrupt_disable() (SPCR &= ~_BV(SPIE))
#define spi0_enable() 					(SPCR |= _BV(SPE))
#define spi0_disable() 					(SPCR &= ~_BV(SPE))
static inline void spi0_config_gpio(void)			{
	gpio_configure(GPIO_SPI0_MISO, GP_INPUT | GP_PULLUP);
	gpio_configure(GPIO_SPI0_MOSI, GP_OUTPUT);
	gpio_configure(GPIO_SPI0_SS, GP_OUTPUT);
	gpio_configure(GPIO_SPI0_SCK, GP_OUTPUT);
}
#define spi0_wait_for_transmit_complete() do {} while(!(SPSR & _BV(SPIF)))
#define spi0_error_collision() (SPSR & _BV(WCOL))

static inline void spi0_init_default(void) {
	spi0_set_mode(SPI_MODE0);
	spi0_config_gpio();
	spi0_interrupt_disable(); 
	spi0_order_msb_first();
	spi0_master();
	spi0_set_clock(SPI_CLOCK_DIV16);
	spi0_enable();
}

static inline void spi0_putc(uint8_t ch) {
	spi0_wait_for_transmit_complete(); 
	SPDR = ch; 
}

static inline uint8_t spi0_getc(void){
	spi0_wait_for_transmit_complete(); 
	return SPDR; 
}

static inline uint8_t spi0_transfer(uint8_t ch) {
	SPDR = ch; 
	spi0_wait_for_transmit_complete();
	return SPDR; 
}

int16_t spi_init_device(uint8_t dev);
uint16_t spi_transfer(uint8_t dev, uint8_t byte);

#endif
