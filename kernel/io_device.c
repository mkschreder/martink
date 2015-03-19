
#include <arch/soc.h>

#include "io_device.h"
#include <errno.h>

enum {
	IO_STATE_CLOSED = 0,
	IO_STATE_OPENING, 
	IO_STATE_OPEN, 
	IO_STATE_PROGRESS, 
	IO_STATE_CLOSING
}; 

#define IO_STATE_MASK 0x07

#define IO_SET_STATE(self, state) ((self)->status = ((self->status) & ~(IO_STATE_MASK)) | ((state) << 0))
#define IO_GET_STATE(self) ((self)->status & IO_STATE_MASK)

#define IO_DEBUG(...) {}

void io_init(struct io_device *self){
	self->api = 0; 
	PT_INIT(&self->thread); 
	self->user_thread = 0; 
	self->size = 0;  
	self->status = 0; 
}

uint8_t io_open(io_dev_t dev, timestamp_t tout){
	struct io_device *self = dev; 
	self->timeout = tout; 
	
	if(self->user_thread != 0 && self->user_thread != libk_current_thread()){
		return 0; 
	}
	
	switch(IO_GET_STATE(self)){
		case IO_STATE_CLOSED: 
			PT_INIT(&self->thread); 
			self->user_thread = libk_current_thread(); 
			IO_SET_STATE(self, IO_STATE_OPENING); 
			IO_DEBUG("io opening\n"); 
		case IO_STATE_OPENING: 
			if(PT_SCHEDULE(self->api->open(&self->thread, dev))) return 0; 
			IO_DEBUG("io open\n"); 
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
	}
	return 0; 
}

uint8_t io_close(io_dev_t dev){
	struct io_device *self = dev; 
	
	if(self->user_thread != libk_current_thread()) return 0; 
	
	switch(IO_GET_STATE(self)){
		case IO_STATE_CLOSED: 
			return 1; 
		case IO_STATE_OPEN: 
			/*if(self->thread.status < PT_EXITED){
				return 0; 
			} */
			PT_INIT(&self->thread); 
			self->error = 0; 
			IO_SET_STATE(self, IO_STATE_CLOSING); 
			IO_DEBUG("io closing\n"); 
		case IO_STATE_CLOSING: 
			if(PT_SCHEDULE(self->api->close(&self->thread, dev))) return 0; 
			IO_DEBUG("io closed\n"); 
			self->user_thread = 0; 
			IO_SET_STATE(self, IO_STATE_CLOSED); 
			return 1; 
	}
	return 0; 
}

static int8_t _io_op(struct io_device *self, ssize_t size){
	if(self->user_thread != libk_current_thread()) {
		self->error = -EACCES; 
		return -1; 
	}
	
	switch(IO_GET_STATE(self)){
		case IO_STATE_OPEN: 
			PT_INIT(&self->thread); 
			self->time = timestamp_from_now_us(self->timeout); 
			self->size = size; 
			self->error = 0; 
			IO_SET_STATE(self, IO_STATE_PROGRESS); 
		case IO_STATE_PROGRESS: {
			if(timestamp_expired(self->time)){
				IO_DEBUG("timed out!\n"); 
				self->error = -ETIMEDOUT; 
				IO_SET_STATE(self, IO_STATE_OPEN); 
				return -1; 
			}
			return 0; 
		}
	}
	return -1; 
}

ASYNC(io_device_t, io_write, const uint8_t *data, ssize_t size)){
	return self->api->write(parent, self, data, size);
}

uint8_t io_write(io_dev_t dev, const uint8_t *data, ssize_t size){
	struct io_device *self = dev; 
	
	switch(_io_op(self, size)){
		case 0: {
			ssize_t offset = size - self->size; 
			if(PT_SCHEDULE(self->api->write(&self->thread, dev, data + offset, size - offset))) return 0; 
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		} break; 
		case -1: {
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		}
	}
	// default is to return a completed write so that higher level code can continue
	return 1; 
}

uint8_t io_read(io_dev_t dev, uint8_t *data, ssize_t size){
	struct io_device *self = dev; 
	
	switch(_io_op(self, size)){
		case 0: {
			ssize_t offset = size - self->size; 
			if(PT_SCHEDULE(self->api->read(&self->thread, dev, data + offset, size - offset))) return 0; 
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		} break; 
		case -1: {
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		}
	}
	return 1; 
}

uint8_t io_seek(io_dev_t dev, ssize_t ofs, int whence){
	struct io_device *self = dev; 
	IO_DEBUG("io seek\n"); 
	
	switch(_io_op(self, self->size)){
		case 0: {
			if(PT_SCHEDULE(self->api->seek(&self->thread, dev, ofs, whence))) return 0; 
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		} break; 
		case -1: {
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		}
	}
	return 0; 
}

uint8_t io_ioctl(io_dev_t dev, ioctl_req_t req, ...){
	struct io_device *self = dev; 
	
	switch(_io_op(self, self->size)){
		case 0: {
			va_list va; 
			va_start(va, req); 
			if(PT_SCHEDULE(self->api->ioctl(&self->thread, dev, req, va))) {
				va_end(va); 
				return 0; 
			}
			va_end(va); 
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		} break; 
		case -1: {
			IO_SET_STATE(self, IO_STATE_OPEN); 
			return 1; 
		}
	}
	return 0; 
}

ssize_t io_error(io_dev_t dev){
	struct io_device *self = dev; 
	return self->error; 
}

ssize_t _io_progress(struct io_device *self, ssize_t count){
	self->size -= count; 
	if(self->size < 0) self->size = 0; 
	return self->size; 
}
