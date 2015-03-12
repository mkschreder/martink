#include <errno.h>
#include <string.h>
#include "rd.h"

#define RAMDISK_FLAG_LOCKED (1 << 0)
#define RAMDISK_FLAG_READ (1 << 1)
#define RAMDISK_FLAG_WRITE (1 << 2)
#define RAMDISK_FLAG_DR (1 << 3)

// simulate an async device
PT_THREAD(_rd_thread(struct libk_thread *kthread, struct pt *pt)){
	struct ramdisk *self = container_of(kthread, struct ramdisk, thread); 
	
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_UNTIL(pt, self->tr_size && (self->status & (RAMDISK_FLAG_READ | RAMDISK_FLAG_WRITE))); 
		
		//printf("RAMDISK: busy..\n"); 
		if(self->status & (RAMDISK_FLAG_READ)){
			printf("RAMDISK: load: %d bytes from %x: ", (int)self->tr_size, (int)self->cur); 
			for(ssize_t c = 0; c < self->tr_size; c++) printf("%x ", (self->buffer + self->cur)[c]); 
			printf("\n"); 
			memcpy(self->cache, self->buffer + self->cur, self->tr_size); 
			self->cur += self->tr_size; 
			self->status |= RAMDISK_FLAG_DR; 
		} else if(self->status & (RAMDISK_FLAG_WRITE)){
			printf("RAMDISK: store: %d bytes to %x: ", (int)self->tr_size, (int)self->cur); 
			for(ssize_t c = 0; c < self->tr_size; c++) printf("%x ", (self->buffer + self->cur)[c]); 
			printf("\n"); 
			memcpy(self->buffer + self->cur, self->cache, self->tr_size); 
			self->cur += self->tr_size; 
			self->tr_size = 0; 
		}
		if(self->cur > self->size) self->cur = self->size; 
		self->status &= ~(RAMDISK_FLAG_READ | RAMDISK_FLAG_WRITE); 
		//printf("RAMDISK: done\n"); 
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
	
	printf("RAMDISK: init: size: %d\n", size); 
	
	libk_create_thread(&self->thread, _rd_thread, "rd"); 
}

static uint8_t _rd_open(block_dev_t dev){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	
	if(self->status & RAMDISK_FLAG_LOCKED) return 0; 
	
	self->status |= RAMDISK_FLAG_LOCKED; 
	self->user_thread = libk_current_thread(); 
	
	//printf("RAMDISK: open\n"); 
	return 1; 
}

static int8_t _rd_close(block_dev_t dev){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(self->user_thread != libk_current_thread()) return -1; 
	
	self->status &= ~RAMDISK_FLAG_LOCKED; 
	self->user_thread = 0; 
	//printf("RAMDISK: close\n"); 
	return 0; 
}

static ssize_t _rd_seek(block_dev_t dev, ssize_t pos, int whence){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	if(self->status & (RAMDISK_FLAG_WRITE | RAMDISK_FLAG_READ)) return -EWOULDBLOCK; 
	
	printf("RAMDISK: seek: %x, %d\n", pos, whence);
	
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
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	if(self->status & (RAMDISK_FLAG_WRITE | RAMDISK_FLAG_READ)) return -EWOULDBLOCK; 
	
	// check if writing is even possible
	if(self->cur >= self->size) return -EOF; 
	
	if(count == 0) return 0; 
	
	// Copy some data to the cache
	if(count > RAMDISK_CACHE_SIZE) count = RAMDISK_CACHE_SIZE; 
	
	memcpy(self->cache, data, count); 
	self->tr_size = count; 
	
	self->status |= RAMDISK_FLAG_WRITE; 
	
	// return number of bytes stored in cache
	return count; 
}

static ssize_t _rd_read(block_dev_t dev, uint8_t *data, ssize_t count){
	struct ramdisk *self = container_of(dev, struct ramdisk, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	if(count > RAMDISK_CACHE_SIZE) count = RAMDISK_CACHE_SIZE; 
	
	if(self->status & (RAMDISK_FLAG_WRITE | RAMDISK_FLAG_READ)) return -EWOULDBLOCK; 
	
	if(count == 0) return 0; 
	
	if(self->status & (RAMDISK_FLAG_DR)){
		if(self->tr_size < count) count = self->tr_size; 
		memcpy(data, self->cache, count); 
		self->status &= ~RAMDISK_FLAG_DR; 
		return count; 
	} else {
		// check if read is allowed
		if(self->cur >= self->size) return -EOF; 
	
		self->tr_size = count; 
		self->status |= RAMDISK_FLAG_READ; 
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
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	switch(req) {
		default: 
			return -EINVAL; 
	}
	return -1; 
}

block_dev_t rd_get_interface(struct ramdisk *self){
	
	static struct block_device _if;
	_if = (struct block_device) {
		.open = _rd_open, 
		.close = _rd_close, 
		.read = 	_rd_read,
		.write = 	_rd_write,
		.seek = _rd_seek, 
		.ioctl = _rd_ioctl, 
	};
	self->api = &_if; 
	return &self->api; 
}
