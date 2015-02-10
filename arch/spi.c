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

#include <kernel/device.h>

struct spi_dev {
	uint8_t id;
	struct serial_if *serial;
	uint16_t data; 
};

static struct spi_dev _spi[] = {
	{.id = 0, .serial = 0},
	{.id = 1, .serial = 0},
	{.id = 2, .serial = 0},
	{.id = 3, .serial = 0}
};

//DECLARE_STATIC_CBUF(spi0_rx_buf, uint8_t, 4); 
//DECLARE_STATIC_CBUF(spi1_rx_buf, uint8_t, 4); 

#define GET_SPI_DEV(s, dev) \
	struct spi_dev *dev = container_of(s, struct spi_dev, serial)
	
static uint16_t _spi_putc(serial_dev_t self, uint8_t ch){
	GET_SPI_DEV(self, dev);
	dev->data = spi_transfer(dev->id, ch); 
	return dev->data; 
}

static uint16_t _spi_getc(serial_dev_t self) {
	GET_SPI_DEV(self, dev);
	uint16_t d = dev->data; 
	dev->data = SERIAL_NO_DATA; 
	return d; 
}

static size_t _spi_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_spi_putc(self, *data++); 
	}
	return size; 
}

static size_t _spi_getn(serial_dev_t self, uint8_t *data, size_t sz){
	size_t count = 0;
	while(sz--){
		*data = _spi_putc(self, 0); 
		data++; 
		count++; 
	}
	return count; 
}

static size_t _spi_waiting(serial_dev_t self){
	GET_SPI_DEV(self, dev);
	return dev->data != SERIAL_NO_DATA; 
}

static int16_t _spi_begin(serial_dev_t self){
	GET_SPI_DEV(self, dev);
	dev->data = SERIAL_NO_DATA; 
	return 0; 
}

static int16_t _spi_end(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

serial_dev_t spi_get_serial_interface(uint8_t dev){
	int count = sizeof(_spi) / sizeof(_spi[0]);
	if(!count) return 0; 
	 
	if(dev >= count) dev = count - 1; 
	
	return &_spi[dev].serial; 
}

/*
void _init(struct device_meta *meta){
	
}

struct serial_device_ops {
	uint16_t (*put)(serial_dev_t dev, uint8_t ch); 
	struct device_ops_meta meta; 
}; 

static struct serial_device_ops _spi_serial SECTION_DEVICE_META = {
	.put = _spi_putc,
}; 

static struct device_ops _spi_ops2 SECTION_DEVICE_META = {
	.type = DEV_SERIAL, 
	.meta = &_spi_serial.meta,
	.next = 0
}; 

static struct device_ops _spi_ops SECTION_DEVICE_META = {
	.type = DEV_SERIAL, 
	.meta = &_spi_serial.meta,
	.next = &_spi_ops2
}; 

DEVICE_INFO(spi[]) = {
	{
		.name = "spi0", 
		.init = _init, 
		.ops = &_spi_ops, 
		.meta = &_spi[0].meta
	}
}; 
*/
void initproc spi_init(void){
	int c; 
	static struct serial_if _if;
	static struct serial_if *i = 0; 
	
	if(!i){
		_if = (struct serial_if) {
			.put = _spi_putc,
			.get = _spi_getc,
			.putn = _spi_putn,
			.getn = _spi_getn,
			.begin = _spi_begin,
			.end = _spi_end, 
			.waiting = _spi_waiting
		}; 
		i = &_if; 
	}
	
	int count = sizeof(_spi) / sizeof(struct spi_dev);
	kdebug("SPI: starting interfaces: ");
	
	for(c = 0; c < count; c++){
		if(spi_init_device(c) == -1) continue; 
		kdebug("%d ", c); 
		_spi[c].id = c; 
		_spi[c].serial = i;
	}
	kdebug("\n");
	/*
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
*/
}
