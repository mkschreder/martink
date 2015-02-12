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

void at24_init(struct at24 *self, i2c_dev_t i2c)
{
	self->i2c = i2c; 
}

static void at24_write_page(struct at24 *self, const uint8_t* data, uint16_t eeaddr, uint8_t count)
{
  uint8_t mbuf[I2C_PacketSize + 2] = {(uint8_t)(eeaddr >> 8), (uint8_t)eeaddr}; 
  uint8_t data_size = (count > I2C_PacketSize)?I2C_PacketSize:count; 
  memcpy(&mbuf[2], data, data_size); 
  i2c_start_write(self->i2c, EEPROM_ADDR, mbuf, 2 + data_size); 
	i2c_stop(self->i2c); 
}

size_t at24_read(struct at24 *self, uint16_t eeaddr, uint8_t* data, size_t count)
{  
	uint8_t mbuf[2] = {(uint8_t)(eeaddr >> 8), (uint8_t)eeaddr}; 
	i2c_start_write(self->i2c, EEPROM_ADDR, mbuf, 2); 
	i2c_start_read(self->i2c, EEPROM_ADDR, data, count); 
	i2c_stop(self->i2c); 
  return 0; 
}

size_t at24_write(struct at24 *self, uint16_t WriteAddr, const uint8_t* pBuffer, size_t NumByteToWrite)
{
	(void)(self); 
  uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0;

  Addr = WriteAddr % I2C_PacketSize;
  count = I2C_PacketSize - Addr;
  NumOfPage =  NumByteToWrite / I2C_PacketSize;
  NumOfSingle = NumByteToWrite % I2C_PacketSize;
 
  /* If WriteAddr is I2C_PacketSize aligned  */
  if(Addr == 0) 
  {
    /* If NumByteToWrite < I2C_PacketSize */
    if(NumOfPage == 0) 
    {
      at24_write_page(self, pBuffer, WriteAddr, NumOfSingle);
      i2c_wait(self->i2c, EEPROM_ADDR);
    }
    /* If NumByteToWrite > I2C_PacketSize */
    else  
    {
      while(NumOfPage--)
      {
        at24_write_page(self, pBuffer, WriteAddr, I2C_PacketSize); 
				i2c_wait(self->i2c, EEPROM_ADDR);
        WriteAddr +=  I2C_PacketSize;
        pBuffer += I2C_PacketSize;
      }

      if(NumOfSingle!=0)
      {
        at24_write_page(self, pBuffer, WriteAddr, NumOfSingle);
        i2c_wait(self->i2c, EEPROM_ADDR);
      }
    }
  }
  /* If WriteAddr is not I2C_PacketSize aligned  */
  else 
  {
    /* If NumByteToWrite < I2C_PacketSize */
    if(NumOfPage== 0) 
    {
      at24_write_page(self, pBuffer, WriteAddr, NumOfSingle);
      i2c_wait(self->i2c, EEPROM_ADDR);
    }
    /* If NumByteToWrite > I2C_PacketSize */
    else
    {
      NumByteToWrite -= count;
      NumOfPage =  NumByteToWrite / I2C_PacketSize;
      NumOfSingle = NumByteToWrite % I2C_PacketSize;	
      
      if(count != 0)
      {  
        at24_write_page(self, pBuffer, WriteAddr, count);
        i2c_wait(self->i2c, EEPROM_ADDR);
        WriteAddr += count;
        pBuffer += count;
      } 
      
      while(NumOfPage--)
      {
        at24_write_page(self, pBuffer, WriteAddr, I2C_PacketSize);
        i2c_wait(self->i2c, EEPROM_ADDR);
        WriteAddr +=  I2C_PacketSize;
        pBuffer += I2C_PacketSize;  
      }
      if(NumOfSingle != 0)
      {
        at24_write_page(self, pBuffer, WriteAddr, NumOfSingle); 
        i2c_wait(self->i2c, EEPROM_ADDR);
      }
    }
  }  
  return 0; 
}

