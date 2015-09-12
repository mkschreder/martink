#include <arch/soc.h>
#include <kernel/gbuf.h>
#include "fb_tty.h"
#include "fb_tty_font_5x7.h"

static void _fb_tty_put(tty_dev_t dev, uint8_t ch, tty_color_t fg, tty_color_t bg){
	struct fb_tty *self = container_of(dev, struct fb_tty, api); 
	const unsigned char *glyph = &tty_font_5x7[ch * 5];
	uint16_t x = self->cursor_x * 6; 
	uint16_t y = self->cursor_y * 8; 
	for(unsigned j = 0; j < 5; j++){
		uint8_t bits = (fg != 0)?glyph[j]:~glyph[j];
		for(unsigned k = 0; k < 8; k++){
			gbuf_draw_pixel(self->gbuf, x + j, y + k, (bits >> k & 1)?fg:bg); 
		}
	}
}

static void _fb_tty_move_cursor(tty_dev_t dev, uint16_t x, uint16_t y){
	struct fb_tty *self = container_of(dev, struct fb_tty, api); 
	//if(x > self->width) x = self->width; 
	//if(y > self->height) y = self->height; 
	self->cursor_x = x; 
	self->cursor_y = y; 
}

static void _fb_tty_get_size(tty_dev_t dev, uint16_t *w, uint16_t *h){
	struct fb_tty *self = container_of(dev, struct fb_tty, api); 
	uint16_t gw = 0, gh = 0; 
	gbuf_get_size(self->gbuf, &gw, &gh); 
	*w = gw / 6; 
	*h = gh / 8; 
}

static void _fb_tty_clear(tty_dev_t dev){
	struct fb_tty *self = container_of(dev, struct fb_tty, api); 
	gbuf_clear(self->gbuf); 
}

void fb_tty_init(struct fb_tty *self, struct gbuf *gbuf){
	self->gbuf = gbuf; 
	self->cursor_x = self->cursor_y = 0; 
}

tty_dev_t fb_tty_to_tty_device(struct fb_tty *self){
	static struct tty_device api = {
		.put = _fb_tty_put, 
		.move_cursor = _fb_tty_move_cursor, 
		.get_size = _fb_tty_get_size, 
		.clear = _fb_tty_clear
	}; 
	self->api = &api; 
	return &self->api; 
}
