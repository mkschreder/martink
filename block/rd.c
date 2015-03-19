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
			RAMDISK_DEBUG("RD: load: %d bytes from %d: ", (int)self->tr_size, (int)self->cur); 
			for(ssize_t c = 0; c < self->tr_size; c++) RAMDISK_DEBUG("%x ", (self->buffer + self->cur)[c]); 
			RAMDISK_DEBUG("\n"); 
			//self->cur += self->tr_size; 
			self->status |= RAMDISK_FLAG_DR; 
		} else if(self->status & (RAMDISK_FLAG_WRITE)){
			RAMDISK_DEBUG("RD: store: %d bytes to %d: ", (int)self->tr_size, (int)self->cur); 
			memcpy(self->buffer + self->cur, self->cache, self->tr_size); 
			for(ssize_t c = 0; c < self->tr_size; c++) RAMDISK_DEBUG("%x ", (self->buffer + self->cur)[c]); 
			RAMDISK_DEBUG("\n"); 
			//self->cur += self->tr_size; 
			self->tr_size = 0; 
			self->status |= RAMDISK_FLAG_TE; 
		}
		if(self->cur > self->size) self->cur = self->size; 
		self->status &= ~(RAMDISK_FLAG_READ | RAMDISK_FLAG_WRITE); 
		RAMDISK_DEBUG("RD: done\n"); 
	}
	PT_END(pt); 
}


static PT_THREAD(_rd_open(struct pt *pt, struct io_device *dev)){
	struct ramdisk *self = container_of(dev, struct ramdisk, io); 
	RAMDISK_DEBUG("RD: open\n"); 
	self->cur = 0; 
	
	PT_BEGIN(pt); 
	PT_END(pt); 
}

static PT_THREAD(_rd_close(struct pt *pt, struct io_device *dev)){
	struct ramdisk *self = container_of(dev, struct ramdisk, io); 
	RAMDISK_DEBUG("RD: close\n"); 
	PT_BEGIN(pt); 
	PT_END(pt); 
}

static PT_THREAD(_rd_seek(struct pt *pt, struct io_device *dev, ssize_t pos, int whence)){
	struct ramdisk *self = container_of(dev, struct ramdisk, io); 
	
	RAMDISK_DEBUG("RD: seek: %x, %d, %d\n", (unsigned int)pos, (unsigned int)pos, whence);
	
	switch(whence){
		case SEEK_SET: self->cur = pos; break; 
		case SEEK_CUR: self->cur += pos; break; 
		case SEEK_END: self->cur = self->size; break; 
	}
	
	if(self->cur < 0) self->cur = 0; 
	if(self->cur > self->size) self->cur = self->size; 
	
	PT_BEGIN(pt); 
	PT_END(pt); 
}

static PT_THREAD(_rd_write(struct pt *pt, struct io_device *dev, const uint8_t *data, ssize_t count)){
	struct ramdisk *self = container_of(dev, struct ramdisk, io); 
	
	// Copy some data to the cache
	if(count > RAMDISK_CACHE_SIZE) count = RAMDISK_CACHE_SIZE; 
	
	
	PT_BEGIN(pt); 
	
	while(1){
		// check if writing is even possible
		if(self->cur >= self->size) {
			RAMDISK_DEBUG("Write past end: %d %d\n", self->cur, self->size); 
			PT_EXIT(pt); 
		}
		
		memcpy(self->cache, data, count); 
		self->tr_size = count; 
		self->status |= RAMDISK_FLAG_WRITE; 
		self->status &= ~RAMDISK_FLAG_TE; 
		
		RAMDISK_DEBUG("RD: write\n"); 
	
		PT_WAIT_UNTIL(pt, self->status & RAMDISK_FLAG_TE); 
		
		self->cur += count; 
		
		if(_io_progress(&self->io, count) == 0){
			break; 
		}
		
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

static PT_THREAD(_rd_read(struct pt *pt, struct io_device *dev, uint8_t *data, ssize_t count)){
	struct ramdisk *self = container_of(dev, struct ramdisk, io); 
	
	if(count > RAMDISK_CACHE_SIZE) count = RAMDISK_CACHE_SIZE; 
	
	
	PT_BEGIN(pt); 
	
	while(1){
		
		if(self->cur >= self->size) {
			RAMDISK_DEBUG("RD: Read past end: %d %d\n", self->cur, self->size); 
			PT_EXIT(pt); 
		}
		
		self->tr_size = count; 
		self->status |= RAMDISK_FLAG_READ; 
		self->status &= ~RAMDISK_FLAG_DR; 
		
		RAMDISK_DEBUG("RD: read\n"); 
	
		PT_WAIT_UNTIL(pt, self->status & RAMDISK_FLAG_DR); 
		
		if(self->tr_size < count) count = self->tr_size; 
		memcpy(data, self->cache, count); 
		
		self->cur += count; 
		
		if(_io_progress(&self->io, count) == 0){
			break; 
		}
		
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}
/*
uint8_t _rd_status(uint8_t dev_id, uint16_t flags){
	if(dev_id >= 1) return 1; 
	uint8_t ret = 0; 
	// IF TWI Interrupt is enabled then the peripheral is busy
	if(flags & I2CDEV_BUSY) ret |= (((TWCR & _BV(TWIE)) != 0) || (TWCR & _BV(TWSTO)))?1:0; 
	return ret; 
}*/

static PT_THREAD(_rd_ioctl(struct pt *pt, struct io_device *dev, ioctl_req_t req, va_list vl)){
	struct ramdisk *self = container_of(dev, struct ramdisk, io); 
	
	PT_BEGIN(pt); 
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
	
	io_init(&self->io); 
	
	libk_create_thread(&self->thread, _rd_thread, "rd"); 
}

io_dev_t rd_get_interface(struct ramdisk *self){
	static struct io_device_ops _if;
	
	_if.open = _rd_open; 
	_if.close = _rd_close; 
	_if.read = 	_rd_read; 
	_if.write = 	_rd_write; 
	_if.seek = _rd_seek; 
	_if.ioctl = _rd_ioctl;
	
	self->io.api = &_if; 
	return &self->io; 
}
