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

#include "errno.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/crc16.h>
#include <util/delay.h>
#include <util/atomic.h>

//typedef char PROGMEM prog_char; 

#include "config.h"

#ifdef CONFIG_HAVE_ADC
#include "mega/adc.h"
#endif

#ifdef CONFIG_HAVE_TWI
#include "mega/twi.h"
#include "mega/twi_slave.h"
#include "mega/twi_slave.h"
#endif

#ifdef CONFIG_HAVE_SPI
#include "mega/spi.h"
#endif

#ifdef CONFIG_HAVE_UART
#include "mega/uart.h"
#endif

#include "mega/stack.h"
#include "mega/time.h"
#include "mega/gpio.h"
#include "mega/pwm.h"
#include "mega/eeprom.h"


#include "eeprom.h"
