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

struct spi_interface {
	/// transmits a byte.
	/// returns byte received while clocking out transmit byte
	/// does not return until operation is completed by hardware
	uint8_t 	(*put_and_getc)(uint8_t byte); 
}; 

#define SPI_DEVICE_INTERFACE(spi_device) (struct spi_interface){\
	.put_and_getc = PFNAME(spi_device, writereadbyte) \
}

void PFDECL(CONFIG_SPI0_NAME, init, void);
uint8_t PFDECL(CONFIG_SPI0_NAME, writereadbyte, uint8_t data);
