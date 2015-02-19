#include "cbuf.h"
#include <arch/soc.h>

#define _cbuf_hmt(name) ((int16_t)(name)->head - (int16_t)(name)->tail)

void cbuf_init(struct cbuf *self, uint8_t *buffer, uint16_t size){
	self->head = self->tail = 0; 
	unsigned bit = 0; 
	for(unsigned c = 0; c < sizeof(size) * 8; c++){
		if(size & 1) bit = c; 
		size >>= 1; 
	}
	self->size = (int16_t)(1 << bit); // make sure we round down to power of two or just 1
	self->buffer = buffer; 
}

void cbuf_clear(struct cbuf *self){
	self->head = self->tail = 0; 
}

uint16_t cbuf_get_waiting(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		int16_t hmt = _cbuf_hmt(self); 
		return (hmt < 0)?(hmt + ((self)->size) + 1):hmt; 
	}
	return 0; 
}

uint16_t cbuf_get_free(struct cbuf *self){
	int16_t s = self->size - cbuf_get_waiting(self) - 1; 
	return s; 
}

int8_t cbuf_is_empty(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return self->head == self->tail; 
	}
	return 0; 
}

int8_t cbuf_is_full(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return ((self)->head == (((self)->tail - 1) & ((self)->size - 1))); 
	}
	return 0; 
}

uint16_t cbuf_get(struct cbuf *self){
	if(cbuf_is_empty(self)) return CBUF_NO_DATA; 
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		self->tail = (self->tail + 1) & (self->size - 1); 
		return self->buffer[self->tail]; 
	}
	return 0; 
}

int8_t cbuf_put(struct cbuf *self, uint8_t data){
	if(cbuf_is_full(self)) return -1; 
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		self->head = (self->head + 1) & (self->size - 1); 
		self->buffer[self->head] = data; 
	}
	return 0; 
}

uint16_t cbuf_getn(struct cbuf *self, uint8_t *data, uint16_t size){
	uint16_t s; 
	for(s = 0; s < size; ++s){
		uint16_t ch = cbuf_get(self); 
		if(ch == CBUF_NO_DATA) break; 
		*data++ = ch; 
	}
	return s; 
}

uint16_t cbuf_putn(struct cbuf *self, const uint8_t *data, uint16_t size){
	uint16_t s = 0; 
	while(s < size){
		if(cbuf_put(self, *data++) == -1) break; 
		++s;  
	}
	return s; 
}

uint8_t cbuf_peek(struct cbuf *self, uint16_t pos){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		int16_t i = (self->tail + pos + 1) & (self->size - 1); 
		return self->buffer[i]; 
	}
	return 0; 
}
