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

/*
extern uint16_t PFDECL(CONFIG_SPI0_NAME, waiting, void); 
extern uint16_t PFDECL(CONFIG_SPI0_NAME, getc, void);
extern void PFDECL(CONFIG_SPI0_NAME, putc, uint8_t data);
extern void PFDECL(CONFIG_SPI0_NAME, puts, const char *s );
extern size_t PFDECL(CONFIG_SPI0_NAME, putn, const char *s, size_t c); 
extern size_t PFDECL(CONFIG_SPI0_NAME, getn, char *s, size_t c); 
*/
//DEFINE_SERIAL_INTERFACE(CONFIG_SPI0_NAME); 

void PFDECL(CONFIG_SPI0_NAME, init, void);
#define spi0_init() PFCALL(CONFIG_SPI0_NAME, init)
