#pragma once 

/* Scratch buffer implementation. Similar to cbuf but handles a single
 * contiguous block of memory. Usually user will modify the underlying buffer
 * himself and then driver would read the underlying data using this structure
 * in order to be able to advance the cursor position as data is being
 * consumed. */

struct sbuf {
	uint8_t *buffer; 
	uint16_t size; 
	uint16_t offset; 
}; 

void sbuf_init(struct sbuf *self, uint8_t *buffer, uint16_t size);
// copies number of bytes from buffer to memory. Does not advance the offset. 
int sbuf_copy(struct sbuf *self, uint8_t *dst, uint16_t size); 
// consumes "count" bytes from the buffer, advancing the offset. 
int sbuf_consume(struct sbuf *self, uint16_t count); 
// rewinds the buffer to the begining 
void sbuf_rewind(struct sbuf *self); 
// copies data from buffer to memory and advances the offset
int sbuf_read(struct sbuf *self, uint8_t *dst, uint16_t size); 
// writes to buffer and advances the offset. 
int sbuf_write(struct sbuf *self, const uint8_t *src, uint16_t size); 
