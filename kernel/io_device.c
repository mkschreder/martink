
#include <arch/soc.h>

#include "io_device.h"
#include <errno.h>
/*
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
*/

#define IO_DEBUG(...) {}

void io_init(struct io_device *self){
	self->api = 0; 
	ASYNC_INIT(&self->vopen); 
	ASYNC_INIT(&self->vclose); 
	ASYNC_INIT(&self->vread); 
	ASYNC_INIT(&self->vwrite); 
	ASYNC_INIT(&self->vseek); 
	ASYNC_INIT(&self->vioctl);
}

ASYNC_PROTOTYPE(io_device_t, open){
	return self->api->open(parent, self); 
}

ASYNC_PROTOTYPE(io_device_t, close){
	return self->api->close(parent, self); 
}

ASYNC_PROTOTYPE(io_device_t, write, const uint8_t *data, ssize_t size){
	return self->api->write(parent, self, data, size); 
}

ASYNC_PROTOTYPE(io_device_t, read, uint8_t *data, ssize_t size){
	return self->api->write(parent, self, data, size); 
}

ASYNC_PROTOTYPE(io_device_t, seek, ssize_t ofs, int whence){
	return self->api->seek(parent, self, ofs, whence); 
}

ASYNC_PROTOTYPE(io_device_t, ioctl, ioctl_req_t req, ...){
	va_list va; 
	va_start(va, req); 
	async_return_t ret = self->api->ioctl(parent, self, req, va); 
	va_end(va); 
	return ret;  
}
