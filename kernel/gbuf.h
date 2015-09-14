#pragma once 

#include <inttypes.h>

typedef enum {
	GBUF_FORMAT_MONOCHROME
} gbuf_format_t; 

typedef uint16_t gbuf_color_t; 

struct gbuf {
	uint8_t *memory;
	uint32_t memsize; 
	gbuf_format_t format; 
	uint16_t width, height;
	uint8_t dirty; 
}; 

void gbuf_init(struct gbuf *self, uint8_t *mem, uint32_t size, uint16_t w, uint16_t h, gbuf_format_t format); 
void gbuf_draw_pixel(struct gbuf *self, uint16_t x, uint16_t y, gbuf_color_t color); 
void gbuf_clear(struct gbuf *self); 
static inline void gbuf_get_size(struct gbuf *self, uint16_t *w, uint16_t *h){ *w = self->width; *h = self->height;}
static inline uint8_t gbuf_is_dirty(struct gbuf *self){ return (self->dirty)?1:0;}
