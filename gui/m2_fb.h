#pragma once

#include <lib/m2gui/m2.h>
#include <lib/m2gui/u8glib/m2ghu8g.h>

#include <kernel/dev/framebuffer.h>
#include <kernel/thread.h>
#include <kernel/cbuf.h>

#ifdef __cplusplus
extern "C" {
#endif

struct m2_fb {
	fb_dev_t 	fbuf; 
	u8g_t 			u8g; 
	uint16_t		width, height; 
	struct cbuf	key_buffer; 
	uint8_t 		key_buffer_data[8]; 
	struct async_process process; 
	timestamp_t time; 
}; 

void m2_fb_init(fb_dev_t _screen, m2_rom_void_p element); 
void m2_fb_put_key(uint8_t key); 

#ifdef __cplusplus
}
#endif
