#include <errno.h>
#include <string.h>
#include "rd.h"

#define RAMDISK_FLAG_LOCKED (1 << 0)
#define RAMDISK_FLAG_READ (1 << 1)
#define RAMDISK_FLAG_WRITE (1 << 2)
#define RAMDISK_FLAG_DR (1 << 3)
#define RAMDISK_FLAG_TE (1 << 4)

#define RAMDISK_DEBUG(...) printf(__VA_ARGS__)

enum {
	RD_READY, 
	RD_WRITE, 
	RD_READ
}; 

// simulate an async device
PT_THREAD(_rd_thread(struct libk_thread *kthread, struct pt *pt)){
	struct ramdisk *self = container_of(kthread, struct ramdisk, thread); 
	
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_UNTIL(pt, self->tr_size && (self->status & (RAMDISK_FLAG_READ | RAMDISK_FLAG_WRITE))); 
		
		RAMDISK_DEBUG("RD: busy..\n"); 
		if(self->status & (RAMDISK_FLAG_READ)){
			memcpy(self->cache, self->buffer + self->cur, self->tr_size); 
			RAMDISK_DEBUG("RD: load: %d bytes from %x: ", (int)self->tr_size, (int)self->cur); 
			for(ssize_t c = 0; c < self->tr_size; c++) RAMDISK_DEBUG("%x ", (self->buffer + self->cur)[c]); 
			RAMDISK_DEBUG("\n"); 
			self->cur += self->tr_size; 
			self->status |= RAMDISK_FLAG_DR; 
		} else if(self->status & (RAMDISK_FLAG_WRITE)){
			RAMDISK_DEBUG("RD: store: %d bytes to %x: ", (int)self->tr_size, (int)self->cur); 
			memcpy(self->buffer + self->cur, self->cache, self->tr_size); 
			for(ssize_t c = 0; c < self->tr_size; c++) RAMDISK_DEBUG("%x ", (self->buffer + self->cur)[c]); 
			RAMDISK_DEBUG("\n"); 
			self->cur += self->tr_size; 
			self->tr_size = 0; 
			self->status |= RAMDISK_FLAG_TE; 
		}
		if(self->cur > self->size) self->cur = self->size; 
		self->status &= ~(RAMDISK_FLAG_READ | RAMDISK_FLAG_WRITE); 
		RAMDISK_DEBUG("RD: done\n"); 
	}
	PT_END(pt); 
}

void rd_init(struct ramdisk *self, uint8_t *buffer, uint16_t size, uint16_t nblocks){
	self->buffer = buffer; 
	self->tr_size = 0; 
	self->size = size; 
	self->nblocks = nblocks; 
	self->status = 0; 
	self->cur = 0; 
	self->state = RD_READY; 
	
	RAMDISK_DEBUG("RD: init: size: %d\n", size); 
	
	block_device_init(&self->base); 
	blk_transfer_init(&self->tr); 
	
	libk_create_thread(&self->thread, _rd_thread, "rd"); 
}


static uint8_t _rd_open(block_dev_t dev){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	RAMDISK_DEBUG("RD: open\n"); 
	return block_device_open(&self->base); 
}

static int8_t _rd_close(block_dev_t dev){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	RAMDISK_DEBUG("RD: close\n"); 
	return block_device_close(&self->base); 
}

static ssize_t _rd_seek(block_dev_t dev, ssize_t pos, int whence){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	RAMDISK_DEBUG("RD: seek: %x, %d, %d\n", (unsigned int)pos, (unsigned int)pos, whence);
	
	switch(whence){
		case SEEK_SET: self->cur = pos; break; 
		case SEEK_CUR: self->cur += pos; break; 
		case SEEK_END: self->cur = self->size; break; 
		default: 
			return -EINVAL; 
	}
	
	if(self->cur < 0) self->cur = 0; 
	if(self->cur > self->size) self->cur = self->size; 
	
	return self->cur; 
}

static ssize_t _rd_write(block_dev_t dev, const uint8_t *data, ssize_t count){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	// Copy some data to the cache
	if(count > RAMDISK_CACHE_SIZE) count = RAMDISK_CACHE_SIZE; 
	
	RAMDISK_DEBUG("RD: write\n"); 
	
	switch(self->state) {
		case RD_READY: 
			// check if writing is even possible
			if(self->cur >= self->size) {
				RAMDISK_DEBUG("Write past end: %d %d\n", self->cur, self->size); 
				return 0; 
			}
			
			//if(count == 0) return 0; 
			
			memcpy(self->cache, data, count); 
			self->tr_size = count; 
			self->status |= RAMDISK_FLAG_WRITE; 
			
			self->state = RD_WRITE; 
			return -EAGAIN; 
		case RD_WRITE: 
			if(self->status & RAMDISK_FLAG_TE){
				self->status &= ~RAMDISK_FLAG_TE; 
				self->state = RD_READY; 
				return count; 
			}
			break; 
	}
	
	return -EAGAIN; 
}

static ssize_t _rd_read(block_dev_t dev, uint8_t *data, ssize_t count){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	if(count > RAMDISK_CACHE_SIZE) count = RAMDISK_CACHE_SIZE; 
	
	RAMDISK_DEBUG("RD: read\n"); 
	
	switch(self->state){
		case RD_READY: 
			if(self->cur >= self->size) {
				RAMDISK_DEBUG("RD: Read past end: %d %d\n", self->cur, self->size); 
				return 0; 
			}
			
			self->tr_size = count; 
			self->status |= RAMDISK_FLAG_READ; 
			self->state = RD_READ; 
			return -EAGAIN; 
		case RD_READ: 
			if(self->status & RAMDISK_FLAG_DR){
				if(self->tr_size < count) count = self->tr_size; 
				memcpy(data, self->cache, count); 
				self->status &= ~RAMDISK_FLAG_DR; 
				self->state = RD_READY; 
				return count; 
			}
			break; 
	} 
	return -EAGAIN; 
}
/*
uint8_t _rd_status(uint8_t dev_id, uint16_t flags){
	if(dev_id >= 1) return 1; 
	uint8_t ret = 0; 
	// IF TWI Interrupt is enabled then the peripheral is busy
	if(flags & I2CDEV_BUSY) ret |= (((TWCR & _BV(TWIE)) != 0) || (TWCR & _BV(TWSTO)))?1:0; 
	return ret; 
}*/

static int16_t _rd_ioctl(block_dev_t dev, ioctl_req_t req, ...){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	return 1; 
}

block_dev_t rd_get_interface(struct ramdisk *self){
	static struct block_device_ops _if;
	
	_if.open = _rd_open; 
	_if.close = _rd_close; 
	_if.read = 	_rd_read; 
	_if.write = 	_rd_write; 
	_if.seek = _rd_seek; 
	_if.ioctl = _rd_ioctl;
	
	self->api = &_if; 
	return &self->api; 
}
