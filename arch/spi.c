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

struct spi_dev {
	uint8_t id;
	struct serial_if *serial;
};

static struct spi_dev _spi[4] = {
	{.id = 0, .serial = 0},
	{.id = 1, .serial = 0},
	{.id = 2, .serial = 0},
	{.id = 3, .serial = 0}
};

DECLARE_STATIC_CBUF(spi0_rx_buf, uint8_t, 4); 
DECLARE_STATIC_CBUF(_sp, uint8_t, 4); 

#define GET_SPI_DEV(s, dev) \
	struct spi_dev *dev = container_of(s, struct spi_dev, serial)
	
uint16_t _spi_putc(serial_dev_t self, uint8_t ch){
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: {
			uint8_t data = spi0_transfer(ch); 
			//cbuf_put(&spi0_rx_buf, data);
			return data; 
			break;
		}
	}
	return 0; 
}

uint16_t _spi_getc(serial_dev_t self) {
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: {
			if(!cbuf_get_data_count(&spi0_rx_buf)) return SERIAL_NO_DATA; 
			int ret = cbuf_get(&spi0_rx_buf); 
			return (ret == -1)?SERIAL_NO_DATA:((uint16_t)ret & 0xff);
			break;
		}
	}
	return SERIAL_NO_DATA; 
}

size_t _spi_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_spi_putc(self, *data++); 
	}
	return size; 
}

size_t _spi_getn(serial_dev_t self, uint8_t *data, size_t sz){
	size_t count = 0;
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: 
			while(sz--){
				*data = hwspi0_transfer(0); 
				data++; 
				count++; 
			}
			break;
	}
	return count; 
}

size_t _spi_waiting(serial_dev_t self){
	GET_SPI_DEV(self, dev);
	switch(dev->id){
		case 0: 
			return cbuf_get_data_count(&spi0_rx_buf); 
			break;
	}
	return 0; 
}

void _spi_flush(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
}

serial_dev_t spi_get_serial_interface(uint8_t dev){
	dev &= 0x03;
	static struct serial_if _if;
	static struct serial_if *i = 0; 
	if(!i){
		_if = (struct serial_if) {
			.put = _spi_putc,
			.get = _spi_getc,
			.putn = _spi_putn,
			.getn = _spi_getn,
			.flush = _spi_flush,
			.waiting = _spi_waiting
		}; 
		i = &_if; 
	}
	_spi[dev].id = dev; 
	_spi[dev].serial = i;
	return &_spi[dev].serial; 
}

void initproc spi_init(void){
	kdebug("SPI: starting interface: ");
	for(int c = 0; c < 4; c++){
		_spi[c].id = c; 
		_spi[c].serial = 0; 
	}
#ifdef CONFIG_HAVE_SPI0
	spi0_init_default(); kdebug("spi0 "); 
#endif
#ifdef CONFIG_HAVE_SPI1
	spi1_init_default(); kdebug("spi1 "); 
#endif
#ifdef CONFIG_HAVE_SPI2
	spi2_init_default(); kdebug("spi2 "); 
#endif
	kdebug("\n");
}
