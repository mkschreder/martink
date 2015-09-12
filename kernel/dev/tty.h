#pragma once

#include <inttypes.h>

typedef struct tty_device **tty_dev_t;
typedef uint16_t tty_color_t;

struct tty_device {
	/// write a character to the screen at current cursor position using the fg and bg colors
	void (*put)(tty_dev_t self, uint8_t ch, tty_color_t fg, tty_color_t bg);
	/// move cursor to a new position on the screen. 
	void (*move_cursor)(tty_dev_t self, uint16_t x, uint16_t y); 
	/// get the size of the screen
	void (*get_size)(tty_dev_t self, uint16_t *w, uint16_t *h); 
	/// clear the screen
	void (*clear)(tty_dev_t self);
}; 

static inline void tty_put(tty_dev_t self, uint8_t ch, tty_color_t fg, tty_color_t bg){
	(*self)->put(self, ch, fg, bg); 
}

static inline void tty_move_cursor(tty_dev_t self, uint16_t x, uint16_t y){
	(*self)->move_cursor(self, x, y); 
}

static inline void tty_get_size(tty_dev_t self, uint16_t *w, uint16_t *h){
	(*self)->get_size(self, w, h); 
}

static inline void tty_clear(tty_dev_t self){
	(*self)->clear(self); 
}
