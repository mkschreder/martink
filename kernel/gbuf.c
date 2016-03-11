#include <arch/soc.h>
//#include <stdlib.h>
//#include <memory.h>
#include <string.h>

#include "gbuf.h"

void gbuf_init(struct gbuf *self, uint8_t *memory, uint32_t memsize, uint16_t width, uint16_t height, gbuf_format_t pform){
	uint32_t size = 0; 
	memset(self, 0, sizeof(struct gbuf)); 
	switch(pform){
		case GBUF_FORMAT_MONOCHROME: 
			size = width * height >> 3; 
			break;
		default: 
			DEBUG("gbuf: unknown pixel format!\n"); 
			break; 
	}
	if(size != memsize) {
		DEBUG("gbuf: wrong memsize (%d), should be %d\n", memsize, size); 
		return; 
	}
	self->width = width; 
	self->height = height; 
	self->format = pform; 
	self->memory = memory;
	self->memsize = memsize; 
}

void gbuf_draw_pixel(struct gbuf *self, uint16_t x, uint16_t y, gbuf_color_t color){
	switch(self->format){
		case GBUF_FORMAT_MONOCHROME: {
			uint32_t i = (y >> 3) * self->width + x;
			if(x >= self->width || y >= self->height || i >= self->memsize) break; 
			uint8_t *byte = &self->memory[(y >> 3) * self->width + x]; 
			if(color) *byte |= 1 << (y & 7); 
			else *byte &= ~(1 << ( y & 7));
			self->dirty = 1; 
		} break;
	}
}

void gbuf_clear(struct gbuf *self) {
	memset(self->memory, 0, self->memsize); 
}
