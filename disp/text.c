#include <arch/soc.h>
#include <string.h>
#include "text.h"

void text_buffer_init(struct text_buffer *self, uint8_t *buffer, uint16_t w, uint16_t h){
	self->buffer = buffer; 
	self->size.w = w; 
	self->size.h = h;
	self->cursor.y = 0; 
	self->cursor.x = 0;  
	memset(self->buffer, ' ', w * h);
}

void text_buffer_render(struct text_buffer *self, serial_dev_t out){
	// go to home
	serial_printf(out, "\x1b[m\x1b[2H"); 
	// hide cursor
	serial_printf(out, "\x1b[?25l"); 
	for(uint32_t c = 0; c < self->size.h; c ++){
		serial_printf(out, "\x1b[%d;%dH", c + 1, 1); 
		for(uint32_t i = 0; i < self->size.w; i++){
			uint8_t ch = self->buffer[c * self->size.w + i]; 
			uint8_t select = ch & 0x80; 
			ch &= ~0x80; 
			if(select){
				serial_printf(out, "\x1b[37;40m"); 
			} else {
				serial_printf(out, "\x1b[47;30m"); 
			}
			serial_putc(out, ch); //self->buffer + c, self->size.w); 
		}
		serial_printf(out, "\x1b[m\x1b[K"); 
	}
	// show cursor
	serial_printf(out, "\x1b[?25h");
}

static void _text_buffer_put(tty_dev_t dev, uint8_t ch, tty_color_t fg, tty_color_t bg){
	struct text_buffer *self = container_of(dev, struct text_buffer, api); 
	(void)(fg); 
	(void)(bg); 
	if(fg) ch |= 0x80; 
	else ch &= ~0x80; 
	self->buffer[self->cursor.y * self->size.w + self->cursor.x] = ch; 
}

static void _text_buffer_move_cursor(tty_dev_t dev, uint16_t x, uint16_t y){
	struct text_buffer *self = container_of(dev, struct text_buffer, api); 
	if(x >= self->size.w) x = self->size.w - 1; 
	if(y >= self->size.h) y = self->size.h - 1; 
	self->cursor.x = x; 
	self->cursor.y = y; 
}

static void _text_buffer_get_size(tty_dev_t dev, uint16_t *w, uint16_t *h){
	struct text_buffer *self = container_of(dev, struct text_buffer, api); 
	*w = self->size.w; 
	*h = self->size.h; 
}

static void _text_buffer_clear(tty_dev_t dev){
	struct text_buffer *self = container_of(dev, struct text_buffer, api); 
	memset(self->buffer, ' ', self->size.w * self->size.h); 
}

static struct tty_device _tty_dev_if = {
	.put = _text_buffer_put, 
	.move_cursor = _text_buffer_move_cursor, 
	.get_size = _text_buffer_get_size, 
	.clear = _text_buffer_clear
}; 

tty_dev_t text_buffer_get_interface(struct text_buffer *self){
	self->api = &_tty_dev_if; 
	return &self->api; 
}
