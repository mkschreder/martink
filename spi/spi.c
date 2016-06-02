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

#include <kernel/list.h>
#include "spi.h"

static LIST_HEAD(_spi_adapters); 

void spi_register_adapter(struct spi_adapter *dev){
	list_add_tail(&dev->list, &_spi_adapters); 
}

struct spi_adapter *spi_get_adapter(int number){
	// TODO: slow method
	struct spi_adapter *dev = NULL, *item; 
	list_for_each_entry(item, &_spi_adapters, list){
		if(!number) { dev = item; break; }
		number--; 
	}
	return dev; 
}
