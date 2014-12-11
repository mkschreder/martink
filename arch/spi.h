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
#pragma once

#include "autoconf.h"

#ifdef CONFIG_HAVE_SPI

// this is the SPI api specification
struct spi_api {
	void (*init)(void); 
	uint8_t (*writereadbyte)(uint8_t); 
}; 

#define SPI_API(device) (struct spi_api){\
	.init = PFNAME(device, init), \
	.writereadbyte = PFNAME(device, writereadbyte)\
}

#define spi0_init() PFCALL(CONFIG_SPI0_NAME, init)
#define spi0_writereadbyte(byte) PFCALL(CONFIG_SPI0_NAME, writereadbyte, byte) 
#endif
