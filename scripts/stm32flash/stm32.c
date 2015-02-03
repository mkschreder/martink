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

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "stm32.h"
#include "utils.h"

#define STM32_ACK	0x79
#define STM32_NACK	0x1F
#define STM32_CMD_INIT	0x7F
#define STM32_CMD_GET	0x00	/* get the version and command supported */
#define STM32_CMD_EE	0x44	/* extended erase */

struct stm32_cmd {
	uint8_t get;
	uint8_t gvr;
	uint8_t gid;
	uint8_t rm;
	uint8_t go;
	uint8_t wm;
	uint8_t er; /* this may be extended erase */
	uint8_t wp;
	uint8_t uw;
	uint8_t rp;
	uint8_t ur;
};

/* device table */
const stm32_dev_t devices[] = {
	{0x412, "Low-density"       , 0x20000200, 0x20002800, 0x08000000, 0x08008000,  4, 1024, 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800},
	{0x410, "Medium-density"    , 0x20000200, 0x20005000, 0x08000000, 0x08020000,  4, 1024, 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800},
        {0x411, "STM32F2xx"         , 0x20002000, 0x20020000, 0x08000000, 0x08100000,  4, 16384, 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF77DF},
        {0x413, "STM32F4xx"         , 0x20002000, 0x20020000, 0x08000000, 0x08100000,  4, 16384, 0x1FFFC000, 0x1FFFC00F, 0x1FFF0000, 0x1FFF77DF},
	{0x414, "High-density"      , 0x20000200, 0x20010000, 0x08000000, 0x08080000,  2, 2048, 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800},
	{0x416, "Medium-density ULP", 0x20000800, 0x20004000, 0x08000000, 0x08020000, 16,  256, 0x1FF80000, 0x1FF8000F, 0x1FF00000, 0x1FF01000},
	{0x418, "Connectivity line" , 0x20001000, 0x20010000, 0x08000000, 0x08040000,  2, 2048, 0x1FFFF800, 0x1FFFF80F, 0x1FFFB000, 0x1FFFF800},
	{0x420, "Medium-density VL" , 0x20000200, 0x20002000, 0x08000000, 0x08020000,  4, 1024, 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800},
	{0x428, "High-density VL"   , 0x20000200, 0x20008000, 0x08000000, 0x08080000,  2, 2048, 0x1FFFF800, 0x1FFFF80F, 0x1FFFF000, 0x1FFFF800},
	{0x430, "XL-density"        , 0x20000800, 0x20018000, 0x08000000, 0x08100000,  2, 2048, 0x1FFFF800, 0x1FFFF80F, 0x1FFFE000, 0x1FFFF800},
        {0x440, "STM32F0xx"         , 0x20000800, 0x20002000, 0x08000000, 0x08010000,  4, 1024, 0x1FFFF800, 0x1FFFF80B, 0x1FFFEC00, 0x1FFFF800},
	{0x0}
};

/* internal functions */
uint8_t stm32_gen_cs(const uint32_t v);
void    stm32_send_byte(const stm32_t *stm, uint8_t byte);
uint8_t stm32_read_byte(const stm32_t *stm);
char    stm32_send_command(const stm32_t *stm, const uint8_t cmd);

/* stm32 programs */
extern unsigned int	stmreset_length;
extern unsigned char	stmreset_binary[];

uint8_t stm32_gen_cs(const uint32_t v) {
	return  ((v & 0xFF000000) >> 24) ^
		((v & 0x00FF0000) >> 16) ^
		((v & 0x0000FF00) >>  8) ^
		((v & 0x000000FF) >>  0);
}

void stm32_send_byte(const stm32_t *stm, uint8_t byte) {	
	serial_err_t err;
	err = serial_write(stm->serial, &byte, 1);
	if (err != SERIAL_ERR_OK) {
		perror("send_byte");
		assert(0);
	}
}

uint8_t stm32_read_byte(const stm32_t *stm) {
	uint8_t byte;
	serial_err_t err;
	err = serial_read(stm->serial, &byte, 1);
	if (err != SERIAL_ERR_OK) {
		perror("read_byte");
		assert(0);
	}
	return byte;
}

char stm32_send_command(const stm32_t *stm, const uint8_t cmd) {
	stm32_send_byte(stm, cmd);
	stm32_send_byte(stm, cmd ^ 0xFF);
	if (stm32_read_byte(stm) != STM32_ACK) {
		fprintf(stderr, "Error sending command 0x%02x to device\n", cmd);
		return 0;
	}
	return 1;
}

stm32_t* stm32_init(const serial_t *serial, const char init) {
	uint8_t len;
	stm32_t *stm;

	stm      = calloc(sizeof(stm32_t), 1);
	stm->cmd = calloc(sizeof(stm32_cmd_t), 1);
	stm->serial = serial;

	if (init) {
		stm32_send_byte(stm, STM32_CMD_INIT);
		if (stm32_read_byte(stm) != STM32_ACK) {
			stm32_close(stm);
			fprintf(stderr, "Failed to get init ACK from device\n");
			return NULL;
		}
	}

	/* get the bootloader information */
	if (!stm32_send_command(stm, STM32_CMD_GET)) return 0;
	len              = stm32_read_byte(stm) + 1;
	stm->bl_version  = stm32_read_byte(stm); --len;
	stm->cmd->get    = stm32_read_byte(stm); --len;
	stm->cmd->gvr    = stm32_read_byte(stm); --len;
	stm->cmd->gid    = stm32_read_byte(stm); --len;
	stm->cmd->rm     = stm32_read_byte(stm); --len;
	stm->cmd->go     = stm32_read_byte(stm); --len;
	stm->cmd->wm     = stm32_read_byte(stm); --len;
	stm->cmd->er     = stm32_read_byte(stm); --len;
	stm->cmd->wp     = stm32_read_byte(stm); --len;
	stm->cmd->uw     = stm32_read_byte(stm); --len;
	stm->cmd->rp     = stm32_read_byte(stm); --len;
	stm->cmd->ur     = stm32_read_byte(stm); --len;
	if (len > 0) {
		fprintf(stderr, "Seems this bootloader returns more then we understand in the GET command, we will skip the unknown bytes\n");
		while(len-- > 0) stm32_read_byte(stm);
	}
	if (stm32_read_byte(stm) != STM32_ACK) {
		stm32_close(stm);
		return NULL;
	}
	
	/* get the version and read protection status  */
	if (!stm32_send_command(stm, stm->cmd->gvr)) {
		stm32_close(stm);
		return NULL;
	}

	stm->version = stm32_read_byte(stm);
	stm->option1 = stm32_read_byte(stm);
	stm->option2 = stm32_read_byte(stm);
	if (stm32_read_byte(stm) != STM32_ACK) {
		stm32_close(stm);
		return NULL;
	}

	/* get the device ID */
	if (!stm32_send_command(stm, stm->cmd->gid)) {
		stm32_close(stm);
		return NULL;
	}
	len = stm32_read_byte(stm) + 1;
	if (len != 2) {
		stm32_close(stm);
		fprintf(stderr, "More then two bytes sent in the PID, unknown/unsupported device\n");
		return NULL;
	}
	stm->pid = (stm32_read_byte(stm) << 8) | stm32_read_byte(stm);
	if (stm32_read_byte(stm) != STM32_ACK) {
		stm32_close(stm);
		return NULL;
	}

	stm->dev = devices;
	while(stm->dev->id != 0x00 && stm->dev->id != stm->pid)
		++stm->dev;

	if (!stm->dev->id) {
		fprintf(stderr, "Unknown/unsupported device (Device ID: 0x%03x)\n", stm->pid);
		stm32_close(stm);
		return NULL;
	}

	return stm;
}

void stm32_close(stm32_t *stm) {
	if (stm) free(stm->cmd);
	free(stm);
}

char stm32_read_memory(const stm32_t *stm, uint32_t address, uint8_t data[], unsigned int len) {
	uint8_t cs;
	unsigned int i;
	assert(len > 0 && len < 257);

	/* must be 32bit aligned */
	assert(address % 4 == 0);

	address = be_u32      (address);
	cs      = stm32_gen_cs(address);

	if (!stm32_send_command(stm, stm->cmd->rm)) return 0;
	assert(serial_write(stm->serial, &address, 4) == SERIAL_ERR_OK);
	stm32_send_byte(stm, cs);
	if (stm32_read_byte(stm) != STM32_ACK) return 0;

	i = len - 1;
	stm32_send_byte(stm, i);
	stm32_send_byte(stm, i ^ 0xFF);
	if (stm32_read_byte(stm) != STM32_ACK) return 0;

	assert(serial_read(stm->serial, data, len) == SERIAL_ERR_OK);
	return 1;
}

char stm32_write_memory(const stm32_t *stm, uint32_t address, uint8_t data[], unsigned int len) {
	uint8_t cs;
	unsigned int i;
	int c, extra;
	assert(len > 0 && len < 257);

	/* must be 32bit aligned */
	assert(address % 4 == 0);

	address = be_u32      (address);
	cs      = stm32_gen_cs(address);

	/* send the address and checksum */
	if (!stm32_send_command(stm, stm->cmd->wm)) return 0;
	assert(serial_write(stm->serial, &address, 4) == SERIAL_ERR_OK);
	stm32_send_byte(stm, cs);
	if (stm32_read_byte(stm) != STM32_ACK) return 0;

	/* setup the cs and send the length */
	extra = len % 4;
	cs = len - 1 + extra;
	stm32_send_byte(stm, cs);

	/* write the data and build the checksum */
	for(i = 0; i < len; ++i)
		cs ^= data[i];

	assert(serial_write(stm->serial, data, len) == SERIAL_ERR_OK);

	/* write the alignment padding */
	for(c = 0; c < extra; ++c) {
		stm32_send_byte(stm, 0xFF);
		cs ^= 0xFF;
	}

	/* send the checksum */
	stm32_send_byte(stm, cs);
	return stm32_read_byte(stm) == STM32_ACK;
}

char stm32_wunprot_memory(const stm32_t *stm) {
	if (!stm32_send_command(stm, stm->cmd->uw)) return 0;
	if (!stm32_send_command(stm, 0x8C        )) return 0;
	return 1;
}

char stm32_erase_memory(const stm32_t *stm, uint8_t spage, uint8_t pages) {
	if (!stm32_send_command(stm, stm->cmd->er)) {
		fprintf(stderr, "Can't initiate chip erase!\n");
		return 0;
	}

	/* The erase command reported by the bootloader is either 0x43 or 0x44 */
	/* 0x44 is Extended Erase, a 2 byte based protocol and needs to be handled differently. */
	if (stm->cmd->er == STM32_CMD_EE) {
 		/* Not all chips using Extended Erase support mass erase */
 		/* Currently known as not supporting mass erase is the Ultra Low Power STM32L15xx range */
 		/* So if someone has not overridden the default, but uses one of these chips, take it out of */
 		/* mass erase mode, so it will be done page by page. This maximum might not be correct either! */
		if (stm->pid == 0x416 && pages == 0xFF) pages = 0xF8; /* works for the STM32L152RB with 128Kb flash */

		if (pages == 0xFF) {
			stm32_send_byte(stm, 0xFF);
			stm32_send_byte(stm, 0xFF); // 0xFFFF the magic number for mass erase
			stm32_send_byte(stm, 0x00); // 0x00 the XOR of those two bytes as a checksum
			if (stm32_read_byte(stm) != STM32_ACK) {
				fprintf(stderr, "Mass erase failed. Try specifying the number of pages to be erased.\n");
				return 0;
			}
			return 1;
		}

		uint16_t pg_num;
		uint8_t pg_byte;
 		uint8_t cs = 0;
 
 		stm32_send_byte(stm, pages >> 8); // Number of pages to be erased, two bytes, MSB first
 		stm32_send_byte(stm, pages & 0xFF);
 
 		for (pg_num = 0; pg_num <= pages; pg_num++) {
 			pg_byte = pg_num >> 8;
 			cs ^= pg_byte;
 			stm32_send_byte(stm, pg_byte);
 			pg_byte = pg_num & 0xFF;
 			cs ^= pg_byte;
 			stm32_send_byte(stm, pg_byte);
 		}
 		stm32_send_byte(stm, 0x00);  // Ought to need to hand over a valid checksum here...but 0 seems to work!
 	
 		if (stm32_read_byte(stm) != STM32_ACK) {
 			fprintf(stderr, "Page-by-page erase failed. Check the maximum pages your device supports.\n");
			return 0;
 		}

 		return 1;
	}

	/* And now the regular erase (0x43) for all other chips */
	if (pages == 0xFF) {
		return stm32_send_command(stm, 0xFF);
	} else {
		uint8_t cs = 0;
		uint8_t pg_num;
		stm32_send_byte(stm, pages-1);
		cs ^= (pages-1);
		for (pg_num = spage; pg_num < (pages + spage); pg_num++) {
			stm32_send_byte(stm, pg_num);
			cs ^= pg_num;
		}
		stm32_send_byte(stm, cs);
		return stm32_read_byte(stm) == STM32_ACK;
	}
}

char stm32_go(const stm32_t *stm, uint32_t address) {
	uint8_t cs;

	address = be_u32      (address);
	cs      = stm32_gen_cs(address);

	if (!stm32_send_command(stm, stm->cmd->go)) return 0;
	serial_write(stm->serial, &address, 4);
	serial_write(stm->serial, &cs     , 1);

	return stm32_read_byte(stm) == STM32_ACK;
}

char stm32_reset_device(const stm32_t *stm) {
	/*
		since the bootloader does not have a reset command, we
		upload the stmreset program into ram and run it, which
		resets the device for us
	*/

	uint32_t length		= stmreset_length;
	unsigned char* pos	= stmreset_binary;
	uint32_t address	= stm->dev->ram_start;
	while(length > 0) {
		uint32_t w = length > 256 ? 256 : length;
		if (!stm32_write_memory(stm, address, pos, w))
			return 0;

		address	+= w;
		pos	+= w;
		length	-= w;
	}

	return stm32_go(stm, stm->dev->ram_start);
}

