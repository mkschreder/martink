#include <inttypes.h>
#include <memory.h>

#include "sbuf.h"

void sbuf_init(struct sbuf *self, uint8_t *buffer, uint16_t size){
	self->buffer = buffer; 
	self->size = size; 
	self->offset = 0; 
}

// copies number of bytes from buffer to memory. Does not advance the offset. 
int sbuf_copy(struct sbuf *self, uint8_t *dst, uint16_t size){
	if(size > (self->size - self->offset)) size = self->size - self->offset; 
	memcpy(dst, self->buffer + self->offset, size); 
	return size; 
}

// consumes "count" bytes from the buffer, advancing the offset. 
int sbuf_consume(struct sbuf *self, uint16_t count){
	if(count > (self->size - self->offset)) count = self->size - self->offset; 
	self->offset += count; 
	return count; 
}

// rewinds the buffer to the begining 
void sbuf_rewind(struct sbuf *self){
	self->offset = 0; 
}

// copies data from buffer to memory and advances the offset
int sbuf_read(struct sbuf *self, uint8_t *dst, uint16_t size){
	int ret = sbuf_copy(self, dst, size); 
	sbuf_consume(self, ret); 
	return ret; 
}

// writes to buffer and advances the offset. 
int sbuf_write(struct sbuf *self, const uint8_t *src, uint16_t size){
	if(size > (self->size - self->offset)) size = self->size - self->offset; 
	if(size){
		memcpy(self->buffer + self->offset, src, size); 
		self->offset += size;
	}
	return size; 
}
