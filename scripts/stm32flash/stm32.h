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


#ifndef _STM32_H
#define _STM32_H

#include <stdint.h>
#include "serial.h"

typedef struct stm32		stm32_t;
typedef struct stm32_cmd	stm32_cmd_t;
typedef struct stm32_dev	stm32_dev_t;

struct stm32 {
	const serial_t		*serial;
	uint8_t			bl_version;
	uint8_t			version;
	uint8_t			option1, option2;
	uint16_t		pid;
	stm32_cmd_t		*cmd;
	const stm32_dev_t	*dev;
};

struct stm32_dev {
	uint16_t	id;
	char		*name;
	uint32_t	ram_start, ram_end;
	uint32_t	fl_start, fl_end;
	uint16_t	fl_pps; // pages per sector
	uint16_t	fl_ps;  // page size
	uint32_t	opt_start, opt_end;
	uint32_t	mem_start, mem_end;
};

stm32_t* stm32_init      (const serial_t *serial, const char init);
void stm32_close         (stm32_t *stm);
char stm32_read_memory   (const stm32_t *stm, uint32_t address, uint8_t data[], unsigned int len);
char stm32_write_memory  (const stm32_t *stm, uint32_t address, uint8_t data[], unsigned int len);
char stm32_wunprot_memory(const stm32_t *stm);
char stm32_erase_memory  (const stm32_t *stm, uint8_t spage, uint8_t pages);
char stm32_go            (const stm32_t *stm, uint32_t address);
char stm32_reset_device  (const stm32_t *stm);

#endif

