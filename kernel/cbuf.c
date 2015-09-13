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
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		self->head = self->tail = 0; 
	}
}

uint16_t cbuf_get_waiting_isr(struct cbuf *self){
	int16_t hmt = _cbuf_hmt(self); 
	return (hmt < 0)?(hmt + ((self)->size)):hmt; 
}

uint16_t cbuf_get_waiting(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return cbuf_get_waiting_isr(self); 
	}
	return 0; 
}

uint16_t cbuf_get_free(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return self->size - cbuf_get_waiting(self) - 1; 
	}
	return 0; 
}

uint8_t cbuf_is_empty(struct cbuf *self){
	//ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return self->head == self->tail; 
	//}
	//return 0; 
}

uint8_t cbuf_is_full_isr(struct cbuf *self){
	return ((self)->head == (((self)->tail - 1) & ((self)->size - 1))); 
}

uint8_t cbuf_is_full(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return cbuf_is_full_isr(self); 
	}
	return 0; 
}

uint16_t cbuf_get_isr(struct cbuf *self){
	if(self->head == self->tail) return CBUF_NO_DATA; 
	self->tail = (self->tail + 1) & (self->size - 1); 
	return self->buffer[self->tail]; 
}

uint16_t cbuf_get(struct cbuf *self){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return cbuf_get_isr(self); 
	}
	return CBUF_NO_DATA; 
}

uint8_t cbuf_put_isr(struct cbuf *self, uint8_t data){
	if(((self)->head == (((self)->tail - 1) & ((self)->size - 1)))) return 0; 
	self->head = (self->head + 1) & (self->size - 1); 
	self->buffer[self->head] = data; 
	return 1; 
}

uint8_t cbuf_put(struct cbuf *self, uint8_t data){
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		return cbuf_put_isr(self, data); 
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
	for(s = 0; s < size; ++s){
		if(!cbuf_put(self, *data)) break;  
		data++; 
	}
	return s; 
}

uint16_t cbuf_peek(struct cbuf *self, uint16_t pos){
	if(cbuf_get_waiting(self) >= pos) return CBUF_NO_DATA; 
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		int16_t i = (self->tail + pos + 1) & (self->size - 1); 
		return self->buffer[i]; 
	}
	return CBUF_NO_DATA; 
}
