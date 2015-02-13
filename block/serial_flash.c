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

#include <arch/soc.h>
#include <block/serial_flash.h>

#define select() (gpio_clear(self->cs_pin), serial_begin(self->port))
#define deselect() (gpio_set(self->cs_pin), serial_end(self->port))
#define write_byte(byte) serial_putc(self->port, (byte) & 0xff)
#define read_status() 
#define write_enabled() (_read_status(self) & M25P16_SR_WEL)
#define write_in_progress() (_read_status(self) & M25P16_SR_WIP)

static uint8_t _read_status(struct serial_flash *self)
{
  uint8_t rv;

  select();
  write_byte(M25P16_I_RDSR);
  rv = write_byte(0);
  deselect();

  return rv;
}

static void _flash_wren(struct serial_flash *self) {
  select();
  write_byte(M25P16_I_WREN);
  deselect();
	
  while(!write_enabled());
}

/*
void _flash_wrdi(struct serial_flash *self) {
  select();
  write_byte(M25P16_I_WRDI);
  deselect();
}
*/

void serial_flash_init(struct serial_flash *self, serial_dev_t port, gpio_pin_t cs_pin) {
  uint8_t i;

	self->port = port; 
	self->cs_pin = cs_pin; 
	
	gpio_configure(cs_pin, GP_OUTPUT); 
	
  select();
  write_byte(M25P16_I_RDID);

  self->props.id = write_byte(0);
  self->props.type = write_byte(0);
  self->props.size = write_byte(0);
  self->props.uid_len = write_byte(0);
  for(i = 0; i < self->props.uid_len; i++) {
    self->props.uid[i] = write_byte(0);
  }
  deselect();
}

/*
static void m25p16_wrsr(uint8_t val){
  _flash_wren(self); 
  
  select();
  ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  bit_bang_write(M25P16_I_WRSR);
  bit_bang_write(val);
  ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
  deselect();
}*/

uint32_t serial_flash_read(struct serial_flash *self, uint32_t addr, uint8_t * buff, uint32_t buff_len){
	uint32_t i; 
	
	// wait for any writes to complete
	while(write_in_progress()); 
	
	select();
	//ENERGEST_ON(ENERGEST_TYPE_FLASH_READ);

	write_byte(M25P16_I_FAST_READ);

	write_byte(addr >> 16); 
	write_byte(addr >> 8); 
	write_byte(addr); 

	// for fast read send a dummy byte
	write_byte(0);

	for(i = 0; i < buff_len; i++) {
		buff[i] = ~write_byte(0);
	}
	//ENERGEST_OFF(ENERGEST_TYPE_FLASH_READ);
	deselect();
	
	return i; 
}

uint32_t serial_flash_write(struct serial_flash *self, uint32_t addr, const uint8_t * buff, uint32_t buff_len)
{
	uint32_t i; 
	// wait for previous write to complete
	while(write_in_progress()); 
	
	// write enable
  _flash_wren(self); 
  
  select();
  //ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  write_byte(M25P16_I_PP);

  // write addr, msb first
  write_byte(addr >> 16); 
	write_byte(addr >> 8); 
	write_byte(addr); 
	
	// write data
  for(i = 0; i < buff_len; i++) {
    write_byte(~buff[i]);
  }
  //ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
  deselect();
  return i; 
}

void serial_flash_sector_erase(struct serial_flash *self, uint8_t s){
  _flash_wren(self); 
  
  select();
  //ENERGEST_ON(ENERGEST_TYPE_FLASH_WRITE);
  write_byte(M25P16_I_SE);
  write_byte(s);
  write_byte(0x00);
  write_byte(0x00);
  deselect();
  
  while(write_in_progress()); 
  
  //ENERGEST_OFF(ENERGEST_TYPE_FLASH_WRITE);
}
/*
void
m25p16_be()
{
  m25p16_wren(); 
  select();
  bit_bang_write(M25P16_I_BE);
  deselect();
}

void
m25p16_dp()
{
  select();
  bit_bang_write(M25P16_I_DP);
  deselect();
}
void
m25p16_res()
{
  select();
  bit_bang_write(M25P16_I_RES);
  deselect();
  
  while(M25P16_WIP());
}

uint8_t
m25p16_res_res()
{
  uint8_t rv;

  select();
  bit_bang_write(M25P16_I_RES);
  bit_bang_write(M25P16_DUMMY_BYTE);
  bit_bang_write(M25P16_DUMMY_BYTE);
  bit_bang_write(M25P16_DUMMY_BYTE);

  rv = bit_bang_read();

  deselect();

  while(M25P16_WIP());
  return rv;
}
*/
