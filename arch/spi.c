/**
	Immediate mode (no interrupts) SPI driver

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

#include "gpio.h"
#include "spi.h"
#include "interface.h"

DECLARE_STATIC_CBUF(spi0_rx_buf, uint8_t, 4); 
//DECLARE_STATIC_CBUF(spi0_tx_buf, uint8_t, 2); 

void PFDECL(CONFIG_SPI0_NAME, init, void) {
	hwspi0_init_default(); 
}

uint16_t _spi0_putc(struct serial_interface *self, uint8_t ch){
	uint8_t data = hwspi0_transfer(ch); 
	cbuf_put(&spi0_rx_buf, data); 
	return 0; 
}

uint16_t _spi0_getc(struct serial_interface *self) {
	if(!cbuf_get_data_count(&spi0_rx_buf)) return SERIAL_NO_DATA; 
	return cbuf_get(&spi0_rx_buf); 
}

size_t _spi0_putn(struct serial_interface *self, const uint8_t *data, size_t sz){
	size_t size = sz; 
	while(sz--){
		hwspi0_transfer(*data++); 
	}
	return size; 
}

size_t _spi0_getn(struct serial_interface *self, uint8_t *data, size_t sz){
	size_t count = 0; 
	while(sz--){
		*data = hwspi0_transfer(0); 
		data++; 
		count++; 
	}
	return count; 
}

size_t PFDECL(CONFIG_SPI0_NAME, waiting, struct serial_interface *self){
	return cbuf_get_data_count(&spi0_rx_buf); 
}

void PFDECL(CONFIG_SPI0_NAME, flush, struct serial_interface *self){
	// do nothing (but may be useful for interrupt driven version) 
}

static void __init spi_init(void){
	kdebug("SPI: starting interface: ");
#ifdef CONFIG_HAVE_SPI0
	spi0_init(); kdebug("0 "); 
#endif
#ifdef CONFIG_HAVE_SPI1
	spi1_init(); kdebug("1 "); 
#endif
#ifdef CONFIG_HAVE_SPI0
	spi2_init(); kdebug("2 "); 
#endif
	kdebug("\n");
}
