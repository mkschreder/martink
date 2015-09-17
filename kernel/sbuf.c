#include <inttypes.h>
#include "sbuf.h"

void sbuf_init(struct sbuf *self, uint8_t *buffer, uint16_t size){
	self->buffer = buffer; 
	self->size = size; 
	self->offset = 0; 
}

// copies number of bytes from buffer to memory. Does not advance the offset. 
int sbuf_copy(struct sbuf *self, uint8_t *dst, uint16_t size){
	if(size > (self->size - self->offset)) size = self->size - self->offset; 
	memcpy(self->buffer + self->offset, dst, size); 
	return size; 
}

// consumes "count" bytes from the buffer, advancing the offset. 
int sbuf_consume(struct sbuf *self, uint16_t count){
	if(count > (self->size - self->offset)) size = self->size - self->offset; 
	self->offset += count; 
}

