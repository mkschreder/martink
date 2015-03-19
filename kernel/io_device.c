
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
	ASYNC_INIT(&self->open); 
	ASYNC_INIT(&self->close); 
	ASYNC_INIT(&self->read); 
	ASYNC_INIT(&self->write); 
	ASYNC_INIT(&self->seek); 
	ASYNC_INIT(&self->ioctl);
}

ASYNC(io_device_t, open){
	ASYNC_BEGIN(); 
	AWAIT(self->api->open(parent, self) == ASYNC_ENDED); 
	ASYNC_END(); 
}

ASYNC(io_device_t, close){
	ASYNC_BEGIN(); 
	AWAIT(self->api->close(parent, self) == ASYNC_ENDED); 
	ASYNC_END(); 
}

ASYNC(io_device_t, write, const uint8_t *data, ssize_t size){
	ASYNC_BEGIN(); 
	AWAIT(self->api->write(parent, self, data, size) == ASYNC_ENDED); 
	ASYNC_END(); 
}

ASYNC(io_device_t, read, uint8_t *data, ssize_t size){
	ASYNC_BEGIN(); 
	AWAIT(self->api->write(parent, self, data, size) == ASYNC_ENDED); 
	ASYNC_END(); 
}

ASYNC(io_device_t, seek, ssize_t ofs, int whence){
	ASYNC_BEGIN(); 
	AWAIT(self->api->seek(parent, self, ofs, whence) == ASYNC_ENDED); 
	ASYNC_END(); 
}

ASYNC(io_device_t, ioctl, ioctl_req_t req, ...){
	va_list va; 
	va_start(va, req); 
	ASYNC_BEGIN(); 
	AWAIT(self->api->ioctl(parent, self, req, va) == ASYNC_ENDED); 
	va_end(va); 
	ASYNC_END();  
}
