#pragma once

#include <kernel/dev/tty.h>

struct fb_tty {
	struct gbuf *gbuf; 
	uint16_t cursor_x, cursor_y; 
	struct tty_device *api; 
}; 

void fb_tty_init(struct fb_tty *self, struct gbuf *gbuf); 
tty_dev_t fb_tty_to_tty_device(struct fb_tty *self); 
