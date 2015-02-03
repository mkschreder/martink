/*
  stm32flash - Open Source ST STM32 flash program for *nix
  Copyright (C) 2010 Geoffrey McRae <geoff@spacevs.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "serial.h"

serial_baud_t serial_get_baud(const unsigned int baud) {
	switch(baud) {
		case   1200: return SERIAL_BAUD_1200  ;
		case   1800: return SERIAL_BAUD_1800  ;
		case   2400: return SERIAL_BAUD_2400  ;
		case   4800: return SERIAL_BAUD_4800  ;
		case   9600: return SERIAL_BAUD_9600  ;
		case  19200: return SERIAL_BAUD_19200 ;
		case  38400: return SERIAL_BAUD_38400 ;
		case  57600: return SERIAL_BAUD_57600 ;
		case 115200: return SERIAL_BAUD_115200;

		default:
			return SERIAL_BAUD_INVALID;
	}
}

unsigned int serial_get_baud_int(const serial_baud_t baud) {
	switch(baud) {
		case SERIAL_BAUD_1200  : return 1200  ;
		case SERIAL_BAUD_1800  : return 1800  ;
		case SERIAL_BAUD_2400  : return 2400  ;
		case SERIAL_BAUD_4800  : return 4800  ;
		case SERIAL_BAUD_9600  : return 9600  ;
		case SERIAL_BAUD_19200 : return 19200 ;
		case SERIAL_BAUD_38400 : return 38400 ;
		case SERIAL_BAUD_57600 : return 57600 ;
		case SERIAL_BAUD_115200: return 115200;

		case SERIAL_BAUD_INVALID:
		default:
			return 0;
	}
}

unsigned int serial_get_bits_int(const serial_bits_t bits) {
	switch(bits) {
		case SERIAL_BITS_5: return 5;
		case SERIAL_BITS_6: return 6;
		case SERIAL_BITS_7: return 7;
		case SERIAL_BITS_8: return 8;

		default:
			return 0;
	}
}

char serial_get_parity_str(const serial_parity_t parity) {
	switch(parity) {
		case SERIAL_PARITY_NONE: return 'N';
		case SERIAL_PARITY_EVEN: return 'E';
		case SERIAL_PARITY_ODD : return 'O';

		default:
			return ' ';
	}
}

unsigned int serial_get_stopbit_int(const serial_stopbit_t stopbit) {
	switch(stopbit) {
		case SERIAL_STOPBIT_1: return 1;
		case SERIAL_STOPBIT_2: return 2;

		default:
			return 0;
	}
}

