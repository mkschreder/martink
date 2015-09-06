#pragma once

#include <kernel/dev/tty.h>

struct text_buffer {
	uint8_t *buffer; 
	struct {
		uint16_t w, h; 
	} size; 
	struct {
		uint16_t x, y; 
	} cursor; 
	struct tty_device *api; 
}; 

void text_buffer_init(struct text_buffer *self, uint8_t *buffer, uint16_t w, uint16_t h); 
tty_dev_t text_buffer_get_interface(struct text_buffer *self); 
void text_buffer_render(struct text_buffer *self, serial_dev_t out); 
