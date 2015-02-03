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


#ifndef _SERIAL_H
#define _SERIAL_H

#include <stdint.h>

typedef struct serial serial_t;

typedef enum {
	SERIAL_PARITY_NONE,
	SERIAL_PARITY_EVEN,
	SERIAL_PARITY_ODD
} serial_parity_t;

typedef enum {
	SERIAL_BITS_5,
	SERIAL_BITS_6,
	SERIAL_BITS_7,
	SERIAL_BITS_8
} serial_bits_t;

typedef enum {
	SERIAL_BAUD_1200,
	SERIAL_BAUD_1800,
	SERIAL_BAUD_2400,
	SERIAL_BAUD_4800,
	SERIAL_BAUD_9600,
	SERIAL_BAUD_19200,
	SERIAL_BAUD_38400,
	SERIAL_BAUD_57600,
	SERIAL_BAUD_115200,

	SERIAL_BAUD_INVALID
} serial_baud_t;

typedef enum {
	SERIAL_STOPBIT_1,
	SERIAL_STOPBIT_2,
} serial_stopbit_t;

typedef enum {
	SERIAL_ERR_OK = 0,

	SERIAL_ERR_SYSTEM,
	SERIAL_ERR_UNKNOWN,
	SERIAL_ERR_INVALID_BAUD,
	SERIAL_ERR_INVALID_BITS,
	SERIAL_ERR_INVALID_PARITY,
	SERIAL_ERR_INVALID_STOPBIT,
	SERIAL_ERR_NODATA
} serial_err_t;

#define CONST

serial_t*    serial_open (const char *device);
void         serial_close(serial_t *h);
void         serial_flush(const serial_t *h);
serial_err_t serial_setup(serial_t *h, const serial_baud_t baud, const serial_bits_t bits, const serial_parity_t parity, const serial_stopbit_t stopbit);
serial_err_t serial_write(const serial_t *h, const void *buffer, unsigned int len);
serial_err_t serial_read (const serial_t *h, const void *buffer, unsigned int len);
const char*  serial_get_setup_str(const serial_t *h);
serial_err_t serial_reset(const serial_t *h, int dtr);

/* common helper functions */
serial_baud_t serial_get_baud      (const unsigned int baud);
unsigned int serial_get_baud_int   (const serial_baud_t baud) CONST;
unsigned int serial_get_bits_int   (const serial_bits_t bits) CONST;
char         serial_get_parity_str (const serial_parity_t parity) CONST;
unsigned int serial_get_stopbit_int(const serial_stopbit_t stopbit) CONST;

#endif
