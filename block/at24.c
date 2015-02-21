/*
 * at24.c - handle most I2C EEPROMs
 *
 * Copyright (C) 2005-2007 David Brownell
 * Copyright (C) 2008 Wolfram Sang, Pengutronix
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <arch/soc.h>
#include "at24.h"

#include <string.h>

#define EEPROM_ADDR 0xA0

#define AT24_SIZE_BYTELEN 5
#define AT24_SIZE_FLAGS 8

#define AT24_BITMASK(x) (BIT(x) - 1)

/* create non-zero magic value for given eeprom parameters */
#define AT24_DEVICE_MAGIC(_len, _flags) 		\
	((1 << AT24_SIZE_FLAGS | (_flags)) 		\
	    << AT24_SIZE_BYTELEN | ilog2(_len))

/*
static const struct i2c_device_id at24_ids[] = {
	{ "24c00", AT24_DEVICE_MAGIC(128 / 8, AT24_FLAG_TAKE8ADDR) },
	{ "24c01", AT24_DEVICE_MAGIC(1024 / 8, 0) },
	{ "24c02", AT24_DEVICE_MAGIC(2048 / 8, 0) },
	{ "spd", AT24_DEVICE_MAGIC(2048 / 8,
		AT24_FLAG_READONLY | AT24_FLAG_IRUGO) },
	{ "24c04", AT24_DEVICE_MAGIC(4096 / 8, 0) },
	{ "24c08", AT24_DEVICE_MAGIC(8192 / 8, 0) },
	{ "24c16", AT24_DEVICE_MAGIC(16384 / 8, 0) },
	{ "24c32", AT24_DEVICE_MAGIC(32768 / 8, AT24_FLAG_ADDR16) },
	{ "24c64", AT24_DEVICE_MAGIC(65536 / 8, AT24_FLAG_ADDR16) },
	{ "24c128", AT24_DEVICE_MAGIC(131072 / 8, AT24_FLAG_ADDR16) },
	{ "24c256", AT24_DEVICE_MAGIC(262144 / 8, AT24_FLAG_ADDR16) },
	{ "24c512", AT24_DEVICE_MAGIC(524288 / 8, AT24_FLAG_ADDR16) },
	{ "24c1024", AT24_DEVICE_MAGIC(1048576 / 8, AT24_FLAG_ADDR16) },
	{ "at24", 0 },
	{  }
};*/

#define I2C_PacketSize           8

#define AT24_STATUS_BUSY (1 << 0)
#define AT24_STATUS_READ (1 << 1)
#define AT24_STATUS_WRITE (1 << 2)

void at24_init(struct at24 *self, i2c_dev_t i2c)
{
	self->i2c = i2c; 
	PT_INIT(&self->thread); 
	PT_INIT(&self->bthread); 
}

int8_t at24_start_write(struct at24 *self, uint16_t eeaddr, const uint8_t* data, uint16_t count){
	if(at24_busy(self)) return -1; 
	self->status |= AT24_STATUS_WRITE; 
	self->status &= ~(AT24_STATUS_READ); 
	self->op.addr = eeaddr; 
	self->op.data = data; 
	self->op.size = count; 
	return 0; 
	/*
  uint8_t mbuf[I2C_PacketSize + 2] = {(uint8_t)(eeaddr >> 8), (uint8_t)eeaddr}; 
  uint8_t data_size = (count > I2C_PacketSize)?I2C_PacketSize:count; 
  memcpy(&mbuf[2], data, data_size); 
  i2c_start_write(self->i2c, EEPROM_ADDR, mbuf, 2 + data_size); 
	i2c_stop(self->i2c); */
}

int8_t at24_start_read(struct at24 *self, uint16_t eeaddr, uint8_t* data, uint16_t count){
	if(at24_busy(self)) return -1; 
	self->status |= AT24_STATUS_READ; 
	self->status &= ~(AT24_STATUS_WRITE); 
	self->op.addr = eeaddr; 
	self->op.data = data; 
	self->op.size = count;
  return 0; 
	/*uint8_t mbuf[2] = {(uint8_t)(eeaddr >> 8), (uint8_t)eeaddr}; 
	i2c_start_write(self->i2c, EEPROM_ADDR, mbuf, 2); 
	i2c_start_read(self->i2c, EEPROM_ADDR, data, count); 
	i2c_stop(self->i2c); */
}

uint8_t at24_busy(struct at24 *self){
	return (self->status & AT24_STATUS_BUSY) != 0; 
}

uint8_t at24_aquire(struct at24 *self){
	if(at24_busy(self)) return 0; 
	self->status |= AT24_STATUS_BUSY; 
	return 1; 
}

void at24_release(struct at24 *self){
	// TODO: correct way to abort a transaction in progress? 
	if(at24_busy(self)) i2c_stop(self->i2c); 
	self->op = {0}; 
	// reset flags
	self->status &= ~(AT24_STATUS_READ | AT24_STATUS_WRITE | AT24_STATUS_BUSY); 
}

PT_THREAD(_at24_data_thread(struct pt *thr, struct at24 *self)){
	uint8_t packet_size = (sizeof(self->op.buffer) - 2); 
	
	PT_BEGIN(thr); 
	
	PT_WAIT_UNTIL(thr, self->status & AT24_STATUS_BUSY); 
	
	while(1) {
		if(self->status & AT24_STATUS_WRITE){
			self->op.buffer[0] = self->op.addr >> 8; 
			self->op.buffer[1] = self->op.addr; 
			self->op.len = (self->op.size > packet_size)?packet_size:self->op.size; 
			self->op.size -= self->op.len; 
			memcpy(self->op.buffer + 2, self->op.data, self->op.len); 
			self->op.data += self->op.len; 
			
			PT_SPAWN(thr, &self->bthread, 
				i2c_write_thread(self->i2c, &self->bthread, EEPROM_ADDR, self->op.buffer, self->op.len)); 
			
			// are we done?
			if(self->op.size == 0){
				self->status &= ~(AT24_STATUS_WRITE | AT24_STATUS_BUSY); 
				
				PT_RESTART(thr); 
			}
		} else if(self->status & AT24_STATUS_READ && self->op.size > 2){
			self->op.data[0] = self->op.addr >> 8; 
			self->op.data[1] = self->op.addr; 
			
			PT_SPAWN(thr, &self->bthread, 
				i2c_write_read_thread(self->i2c, &self->bthread, EEPROM_ADDR, self->op.data, 2, self->op.size)); 
			
			self->status &= ~(AT24_STATUS_WRITE | AT24_STATUS_BUSY); 
			
			PT_RESTART(thr); 
		} else {
			PT_YIELD(thr); 
		}
	}
	
	PT_END(thr); 
}

void at24_update(struct at24 *self){
	_at24_data_thread(&self->thread, self); 
}

/*
uint16_t at24_start_write(struct at24 *self, 
	uint16_t WriteAddr, const uint8_t* pBuffer, uint16_t NumByteToWrite)
{
	(void)(self); 
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

  Addr = WriteAddr % I2C_PacketSize;
  count = I2C_PacketSize - Addr;
  NumOfPage =  NumByteToWrite / I2C_PacketSize;
  NumOfSingle = NumByteToWrite % I2C_PacketSize;
 
  
  if(Addr == 0) 
  {
    if(NumOfPage == 0) 
    {
      at24_write_page(self, pBuffer, WriteAddr, NumOfSingle);
      //i2c_wait(self->i2c, EEPROM_ADDR);
    }
    else  
    {
      while(NumOfPage--)
      {
        at24_write_page(self, pBuffer, WriteAddr, I2C_PacketSize); 
				//i2c_wait(self->i2c, EEPROM_ADDR);
        WriteAddr +=  I2C_PacketSize;
        pBuffer += I2C_PacketSize;
      }

      if(NumOfSingle!=0)
      {
        at24_write_page(self, pBuffer, WriteAddr, NumOfSingle);
        //i2c_wait(self->i2c, EEPROM_ADDR);
      }
    }
  }
  else 
  {
    if(NumOfPage== 0) 
    {
      at24_write_page(self, pBuffer, WriteAddr, NumOfSingle);
      //i2c_wait(self->i2c, EEPROM_ADDR);
    }
    else
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / I2C_PacketSize;
      NumOfSingle = NumByteToWrite % I2C_PacketSize;	
      
      if(count != 0)
      {  
        at24_write_page(self, pBuffer, WriteAddr, count);
        //i2c_wait(self->i2c, EEPROM_ADDR);
        WriteAddr += count;
        pBuffer += count;
      } 
      
      while(NumOfPage--)
      {
        at24_write_page(self, pBuffer, WriteAddr, I2C_PacketSize);
        //i2c_wait(self->i2c, EEPROM_ADDR);
        WriteAddr +=  I2C_PacketSize;
        pBuffer += I2C_PacketSize;  
      }
      if(NumOfSingle != 0)
      {
        at24_write_page(self, pBuffer, WriteAddr, NumOfSingle); 
        //i2c_wait(self->i2c, EEPROM_ADDR);
      }
    }
  }  
  return 0; 
}
*/
