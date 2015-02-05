/*
 * This file is part of martink project. 
 *
 * Copyright (c) 2010, Loughborough University - Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/**
 * \file
 *         Header file for the control of the M25P16 on sensinode N740s.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 */

#ifndef M25P16_H_
#define M25P16_H_

/* Instruction Set */
#define M25P16_I_WREN      0x06 /* Write Enable */
#define M25P16_I_WRDI      0x04 /* Write Disable */
#define M25P16_I_RDID      0x9F /* Read Identification */
#define M25P16_I_RDSR      0x05 /* Read Status Register */
#define M25P16_I_WRSR      0x01 /* Write Status Register */
#define M25P16_I_READ      0x03 /* Read Data Bytes */
#define M25P16_I_FAST_READ 0x0B /* Read Data Bytes at Higher Speed */
#define M25P16_I_PP        0x02 /* Page Program */
#define M25P16_I_SE        0xD8 /* Sector Erase */
#define M25P16_I_BE        0xC7 /* Bulk Erase */
#define M25P16_I_DP        0xB9 /* Deep Power-down */
#define M25P16_I_RES       0xAB /* Release from Deep Power-down */

/* Status Register Bits */
#define M25P16_SR_SRWD     0x80 /* Status Register Write Disable */
#define M25P16_SR_BP2      0x10 /* Block Protect 2 */
#define M25P16_SR_BP1      0x08 /* Block Protect 1 */
#define M25P16_SR_BP0      0x04 /* Block Protect 0 */
#define M25P16_SR_BP       0x1C /* All Block Protect Bits */
#define M25P16_SR_WEL      0x02 /* Write Enable Latch */
#define M25P16_SR_WIP      0x01 /* Write in Progress */

/* Do we use READ or FAST_READ to read? Fast by default */
#ifdef M25P16_CONF_READ_FAST
#define M25P16_READ_FAST M25P16_CONF_READ_FAST
#else
#define M25P16_READ_FAST 1
#endif

struct serial_flash_props {
  uint8_t id;    /** Manufacturer ID */
  uint8_t type;  /** Memory Type */
  uint8_t size;  /** Memory Size */
  uint8_t uid_len;   /** Unique ID length */
  uint8_t uid[16];   /** Unique ID */
};

struct serial_flash {
	serial_dev_t 	port; 
	gpio_pin_t 		cs_pin; 
	struct serial_flash_props props; 
}; 

void serial_flash_init(struct serial_flash *self, serial_dev_t port, gpio_pin_t cs_pin); 
uint32_t serial_flash_write(struct serial_flash *self, uint32_t addr, const uint8_t *data, uint32_t count); 
uint32_t serial_flash_read(struct serial_flash *self, uint32_t addr, uint8_t *data, uint32_t count); 
void serial_flash_sector_erase(struct serial_flash *self, uint8_t s); 

#endif /* M25P16_H_ */
