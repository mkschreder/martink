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

void time_init(void); 
void uart_init(void); 
void gpio_init(void); 
void spi_init(void); 
void twi_init(void); 
void pwm_init(void); 

void soc_init(void){
	timestamp_init(); 
	time_init(); 
	uart_init(); 
	gpio_init();
	spi_init(); 
	twi_init(); 
	pwm_init(); 
}
