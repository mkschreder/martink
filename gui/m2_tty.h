#pragma once

#include <lib/m2gui/m2.h>
#include <lib/m2gui/u8glib/m2ghu8g.h>

#include <kernel/thread.h>
#include <kernel/dev/tty.h>
#include <kernel/cbuf.h>

#ifdef __cplusplus
extern "C" {
#endif


struct m2_tty {
	serial_dev_t screen; 
	m2_t 				m2; 
	uint16_t		width, height; 
	struct cbuf	key_buffer; 
	uint8_t 		key_buffer_data[8]; 
	struct libk_thread thread; 
	timestamp_t time; 
}; 

void m2_tty_init(struct m2_tty *self, serial_dev_t _screen, uint16_t w, uint16_t h, m2_rom_void_p element); 
void m2_tty_put_key(struct m2_tty *self, uint8_t key); 

#ifdef __cplusplus
}
#endif
