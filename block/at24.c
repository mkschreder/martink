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
#include <errno.h>

#include "at24.h"

#include <string.h>

#define EEPROM_ADDR 0xA0

#define AT24_SIZE_BYTELEN 5
#define AT24_SIZE_FLAGS 8

#define AT24_BITMASK(x) (BIT(x) - 1)

//#define AT24_DEBUG(...) /*do {} while(0) */ printf(__VA_ARGS__)
#define AT24_DEBUG(...) do {} while(0)

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

#define AT24_READ_TIMEOUT_US 1000000UL

/*
static PT_THREAD(_at24_write_thread(struct pt *thr, struct at24 *self)){
	const uint8_t packet_size = (sizeof(self->op.buffer) - 2); 
	
	PT_BEGIN(thr); 
	
	while(1){
		self->op.len = (self->op.size > packet_size)?packet_size:self->op.size; 
		self->op.size -= self->op.len; 
		memcpy(self->op.buffer + 2, self->op.wr_data, self->op.len); 
		
		PT_SPAWN(thr, &self->bthread, 
			i2c_write_thread(self->i2c, &self->bthread, EEPROM_ADDR, self->op.buffer, self->op.len)); 
		
		self->op.wr_data += self->op.len; 
		
		if(self->op.size == 0) PT_EXIT(thr); 
	}
	
	PT_END(thr); 
}
*/
// main io thread that shuffles data between the queued operation and the device
/*
static PT_THREAD(_at24_thread(struct libk_thread *kthread, struct pt *pt)){
	struct at24 *self = container_of(kthread, struct at24, thread); 
	
	PT_BEGIN(pt); 
	
	while(1) {
		// wait until an operation is pending
		PT_WAIT_UNTIL(pt, self->status & AT24_FLAG_BUSY); 
		
		AT24_DEBUG("AT24: processing request\n"); 
		
		if(self->status & AT24_FLAG_READ){
			//wait for the i2c device to be ready
			PT_WAIT_UNTIL(pt, i2c_open(self->i2c)); 
			
			AT24_DEBUG("AT24: writing address\n"); 
			
			i2c_write(self->i2c, EEPROM_ADDR, self->op.buffer, AT24_ADDRESS_SIZE); 
			PT_WAIT_UNTIL(pt, i2c_status(self->i2c, I2CDEV_READY)); 
			
			AT24_DEBUG("AT24: reading %d bytes\n", self->op.size); 
			
			// wait until all bytes are received. 
			self->op.timeout = timestamp_from_now_us(AT24_READ_TIMEOUT_US); 
			self->op.result = 0; 
			
			do {
				ssize_t ret = i2c_read(self->i2c, EEPROM_ADDR, self->op.buffer + self->op.result, self->op.size - self->op.result); 
				if(ret > 0) self->op.result += ret; 
				if(self->op.result == self->op.size){
					break; 
				}
				if(timestamp_expired(self->op.timeout)) {
					AT24_DEBUG("AT24: request timed out!\n"); 
					self->op.error = -ETIMEDOUT; 
					break; 
				}
				PT_YIELD(pt); 
			} while(self->op.result < self->op.size); 
			
			AT24_DEBUG("AT24: read completed\n"); 
			
			// then close the device
			i2c_close(self->i2c); 
			PT_WAIT_WHILE(pt, i2c_status(self->i2c, I2CDEV_BUSY)); 
			
			AT24_DEBUG("AT24: i2c device closed\n"); 
			self->status &= ~(AT24_FLAG_READ | AT24_FLAG_BUSY); 
		} else if(self->status & AT24_FLAG_WRITE){
			PT_WAIT_UNTIL(pt, i2c_open(self->i2c));
			
			AT24_DEBUG("AT24: writing %d bytes\n", self->op.size); 
			self->op.result = 0; 
			while(self->op.result < self->op.size){
				ssize_t ret = i2c_write(self->i2c, EEPROM_ADDR, self->op.buffer + self->op.result, self->op.size - self->op.result); 
				if(ret > 0){
					self->op.result += ret; 
				}
				PT_YIELD(pt); 
			}
			
			PT_WAIT_UNTIL(pt, i2c_status(self->i2c, I2CDEV_READY)); 
			AT24_DEBUG("AT24: write done. closing i2c\n"); 
			
			i2c_close(self->i2c);
			PT_WAIT_WHILE(pt, i2c_status(self->i2c, I2CDEV_BUSY)); 
			
			AT24_DEBUG("AT24: i2c closed\n"); 
			self->status &= ~(AT24_FLAG_WRITE | AT24_FLAG_BUSY); 
		} 
		PT_YIELD(pt);  
	}
	
	PT_END(pt); 
}
*/
void at24_init(struct at24 *self, block_dev_t i2c)
{
	i2cblk_init(&self->i2cblk, i2c, EEPROM_ADDR); 
	self->i2cdev = i2cblk_get_interface(&self->i2cblk); 
	blk_ioctl(self->i2cdev, I2CBLK_SET_AW, 2); // set 16 bit address width
	//blk_transfer_reset(&self->tr); 
	//self->status = 0; 
	//self->op = (struct at24_op){0}; 
	//libk_create_thread(&self->thread, _at24_thread, "at24"); 
}
/*
ssize_t at24_seek(struct at24 *self, ssize_t pos, int from){
	switch(from){
		case SEEK_SET: self->cur = 0; break; 
		case SEEK_END: self->cur = self->size; break; 
		case SEEK_CUR: self->cur += pos; break; 
	}
	return self->cur; 
}

ssize_t at24_write(struct at24 *self, const uint8_t* data, ssize_t count){
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	if(self->status & AT24_FLAG_BUSY) return -EBUSY; 
	
	self->status |= AT24_FLAG_BUSY; 
	if(count > sizeof(self->buffer)) count = sizeof(self->buffer); 
	
	memcpy(self->buffer, data, count); 
	
	blk_transfer_init(&self->tr, i2cblk_get_interface(&self->i2cblk), self->cur, self->buffer, count, IO_WRITE); 
	
	return count; 
}

ssize_t at24_read(struct at24 *self, uint8_t* data, ssize_t count){
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	// reads only fail if write is in progress. Otherwise bytes can be read continuously as they arrive. 
	if(at24_get_status(self, AT24_FLAG_WRITE)) return -EBUSY; 
	
	if(!(self->status & AT24_FLAG_BUSY)){
		self->status |= (AT24_FLAG_BUSY | AT24_FLAG_READ); 
		self->status &= ~(AT24_FLAG_WRITE); 
		
		// place address first
		self->op.buffer[0] = eeaddr >> 8; 
		self->op.buffer[1] = eeaddr; 
		
		self->op.size = (count > AT24_PAGE_SIZE)?AT24_PAGE_SIZE:count;

		return 0; 
	} else if(self->op.result >= count){
		ssize_t c = (self->op.result > count)?count:self->op.result; 
		memcpy(data, self->op.buffer, c); 
		self->op.result -= c; 
		return c; 
	} else {
		return 0; 
	}
}

uint8_t at24_get_status(struct at24 *self, at24_flag_t flags){
	return (self->status & flags) == flags; 
}

uint8_t at24_open(struct at24 *self){
	if(self->status & AT24_FLAG_LOCKED) return 0; 
	self->status |= AT24_FLAG_LOCKED; 
	self->user_thread = libk_current_thread(); 
	return 1; 
}

int8_t at24_close(struct at24 *self){
	if(self->user_thread != libk_current_thread()) return -1; 
	
	self->op = (struct at24_op){0}; 
	self->user_thread = 0; 
	// reset flags
	self->status &= ~(AT24_FLAG_READ | AT24_FLAG_WRITE | AT24_FLAG_BUSY | AT24_FLAG_LOCKED); 
	
	return 0; 
}

int8_t at24_get_geometry(struct at24 *self, struct block_device_geometry *geom){
	(void)self; 
	geom->page_size = AT24_PAGE_SIZE; 
	geom->sectors = 1; 
	geom->pages = geom->sectors * 256; 
	return 0; 
}
*/
static uint8_t _at24_open(block_dev_t self){
	struct at24 *at = container_of(self, struct at24, dev); 
	return blk_open(at->i2cdev); 
}

static int8_t _at24_close(block_dev_t self){
	struct at24 *at = container_of(self, struct at24, dev); 
	return blk_close(at->i2cdev); 
}

static ssize_t _at24_seek(block_dev_t self, ssize_t pos, int from){
	struct at24 *at = container_of(self, struct at24, dev);
	ssize_t cur = blk_seek(at->i2cdev, 0, SEEK_CUR); 
	if(cur < 0) return cur; 
	
	switch(from){
		case SEEK_SET: 
			if(pos > at->size) return -EFBIG; 
			break; 
		case SEEK_END: 
			if(pos > 0) return -EFBIG; 
			else if(at->size + pos < 0) return -EFBIG; 
			break; 
		case SEEK_CUR: 
			if(cur + pos > at->size) return -EFBIG; 
			break;  
	}

	return blk_seek(at->i2cdev, pos, from); 
}

/// copies a page into the internal cache
static ssize_t _at24_write(block_dev_t self, const uint8_t *data, ssize_t data_size){
	struct at24 *at = container_of(self, struct at24, dev); 
	return blk_write(at->i2cdev, data, data_size); 
}

/// reads a page from the internal cache if available
static ssize_t _at24_read(block_dev_t self, uint8_t *data, ssize_t data_size){
	struct at24 *at = container_of(self, struct at24, dev); 
	return blk_read(at->i2cdev, data, data_size); 
}
/*
static int8_t _at24_get_geometry(block_dev_t self, struct block_device_geometry *geom){
	//struct at24 *at = container_of(self, struct at24, dev); 
	(void)self; 
	geom->page_size = AT24_PAGE_SIZE; 
	geom->sectors = 1; 
	geom->pages = geom->sectors * 256; 
	return 0; 
}
*/
static int16_t _at24_ioctl(block_dev_t dev, ioctl_req_t req, ...){
	(void)dev; 
	(void)req; 
	//struct at24 *self = container_of(dev, struct at24, dev); 
	return -EINVAL; 
}

block_dev_t at24_get_block_device_interface(struct at24 *self){
	static struct block_device _if;
	static struct block_device *i = 0; 
	
	if(!i){
		_if = (struct block_device) {
			.open = _at24_open, 
			.close = _at24_close, 
			.read = _at24_read,
			.write = _at24_write,
			.seek = _at24_seek, 
			.ioctl = _at24_ioctl, 
			//.get_geometry = _at24_get_geometry,
			//.get_status = _at24_get_status
		}; 
		i = &_if; 
	}
	self->dev = i; 
	return &self->dev; 
}

/*
/// waits until eeprom is ready and then waits until write is completed
uint16_t at24_blocking_write(struct at24 *self, uint16_t addr, const uint8_t *buf, uint16_t count){
	(void)self; (void)addr; (void)buf; (void)count; 
	// WRONG
	while(!at24_aquire(self)) at24_update(self); 
	at24_start_write(self, addr, buf, count); 
	//while(at24_busy(self)) at24_update(self); 
	//at24_release(self); 
	return count; 
}

/// waits until eeprom is ready and then waits until read is completed
uint16_t at24_blocking_read(struct at24 *self, uint16_t addr, uint8_t *buf, uint16_t count){
	(void)self; (void)addr; (void)buf; (void)count; 
	while(!at24_aquire(self)) at24_update(self); 
	at24_start_read(self, addr, buf, count); 
	//while(at24_busy(self)) at24_update(self); 
	//at24_release(self); 
	return count;
}
*/
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
