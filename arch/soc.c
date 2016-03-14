/**
	This file is part of martink kernel library

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

#include "soc.h"
#include <stdarg.h>
#include <stdio.h>

#if 0
uint16_t serial_printf(serial_dev_t port, const char *fmt, ...){
	char buffer[64]; 
	uint16_t n; 
	va_list vl; 
	if(!port) return 0; 
	
	va_start(vl, fmt);
	n = vsnprintf(buffer, sizeof(buffer), fmt, vl); 
	va_end(vl);
	
	serial_putn(port, (uint8_t*)buffer, n); 
	return n; 
}
#endif
