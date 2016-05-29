/**
	Fast macro based SPI interface for AVR Mega 328P

	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

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
	Github: https://github.com/mkschreder
*/

#include <avr/io.h>
#include <arch/soc.h>

#include "spi.h"

#include <gpio/gpio.h>
#include <gpio/atmega_gpio.h>

#include <kernel/mt.h>

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
#define spi0_interrupt_on() (SPCR |= _BV(SPIE))
#define spi0_interrupt_off() (SPCR &= ~_BV(SPIE))
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
	spi0_interrupt_off(); 
	spi0_order_msb_first();
	spi0_master();
	spi0_set_clock(SPI_CLOCK_DIV16);
	spi0_enable();
}

struct atmega_spi {
	struct spi_adapter device; 
	mutex_t lock; 
	mutex_t ready; 
	char *data; 
	uint16_t size; 
	uint16_t count; 
}; 

static struct atmega_spi spi0; 

ISR(SPI_STC_vect){
	if(spi0.data && spi0.count <= spi0.size)	{
		// store received byte in the previous position
		if(spi0.count > 0) spi0.data[spi0.count - 1] = SPDR; 
		// send current position, but only if not done yet
		if(spi0.count != spi0.size) SPDR = spi0.data[spi0.count]; 
		// signal completed transaction
		if(spi0.count == spi0.size) {
			spi0.data = 0; 
			spi0.size = 0; 
			mutex_unlock_from_isr(&spi0.ready); 
			return; 
		}
		spi0.count++; 
	} 
}

static int atmega_transfer(struct spi_adapter *dev, char *data, size_t size){
	mutex_lock(&spi0.lock); 
	spi0_interrupt_off(); 
	spi0.data = data + 1; 
	spi0.size = size; 
	spi0.count = 1; 	
	SPDR = data[0]; 
	spi0_interrupt_on(); 
	mutex_lock(&spi0.ready); 
	mutex_unlock(&spi0.lock); 
	return spi0.count; 
}

static struct spi_adapter_ops atmega_spi_ops = {
	.transfer = atmega_transfer
}; 

struct spi_adapter *atmega_spi_get_adapter(void){
	memset(&spi0, 0, sizeof(spi0)); 
	mutex_init(&spi0.lock); 
	mutex_init(&spi0.ready); 
	mutex_lock(&spi0.ready); 
	spi0.device.ops = &atmega_spi_ops; 
	spi0_init_default(); 
	spi0_interrupt_on(); 
	
	return &spi0.device; 
}
