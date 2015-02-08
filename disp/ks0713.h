/*
 *                  Copyright 2014 ARTaylor.co.uk
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * Author: Richard Taylor (richard@artaylor.co.uk)
 */

#ifndef _LCD_H
#define _LCD_H

#include <inttypes.h>

struct ks0713_interface {
	void (*write_word)(struct ks0713_interface *self, uint16_t word); 
}; 

struct ks0713 {
	struct ks0713_interface *gpio; 
	uint16_t port_state; 
}; 

typedef enum {
	KS0713_OP_NONE, 
	KS0713_OP_SET, 
	KS0713_OP_CLR,
	KS0713_OP_XOR
} ks0713_pixel_op_t; 

void ks0713_init(struct ks0713 *self, struct ks0713_interface *gpio);
void ks0713_set_backlight(struct ks0713 *self, uint8_t on);
void ks0713_set_contrast(struct ks0713 *self, uint8_t val);
void ks0713_commit(struct ks0713 *self);
void ks0713_move_cursor(struct ks0713 *self, uint8_t x, uint8_t y);
void ks0713_draw_pixel(struct ks0713 *self, uint8_t x, uint8_t y, ks0713_pixel_op_t op);
//void ks0713_putc(struct ks0713 *self, uint8_t ch, uint16_t flags);
//void ks0713_puts(const char *s, LCD_OP op, uint16_t flags);
void ks0713_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, ks0713_pixel_op_t op);
void ks0713_draw_rect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, ks0713_pixel_op_t op);

#endif // _LCD_H
