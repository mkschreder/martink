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

#include "interface.h"

#define KS0713_WIDTH		128
#define KS0713_HEIGHT		64

// pin positions as they are outputted by the driver
#define KS0713_PIN_MASK	(0x1FFF)

#define KS0713_DATA		(0xFF)		// D0-D7
#define KS0713_RD			(1 << 8)	// RD / E
#define KS0713_WR			(1 << 9)	// RD / #WR
#define KS0713_A0			(1 << 10)	// A0 / RS / Data / #CMD
#define KS0713_RES			(1 << 11)	// Reset
#define KS0713_CS1			(1 << 12)	// Chip Select 1

#define KS0713_BACKLIGHT	(1 << 13)

#define KS0713_COMMAND_BUFFER_SIZE (KS0713_WIDTH / 8 * 2)

struct ks0713 {
	uint16_t port_state; 
	uint8_t contrast;
	uint8_t lcd_buffer[KS0713_WIDTH * KS0713_HEIGHT / 8];
	uint16_t cursor_x, cursor_y; 
	
	void (*putn)(struct ks0713 *self, uint16_t *data, size_t size); 
	
	struct tty_device *tty; 
}; 

typedef enum {
	KS0713_OP_NONE, 
	KS0713_OP_SET, 
	KS0713_OP_CLR,
	KS0713_OP_XOR
} ks0713_pixel_op_t; 

void ks0713_init(struct ks0713 *self, void (*putn)(struct ks0713 *self, uint16_t *data, size_t size));
void ks0713_set_backlight(struct ks0713 *self, uint8_t on);
void ks0713_set_contrast(struct ks0713 *self, uint8_t val);
void ks0713_clear(struct ks0713 *self);
void ks0713_commit(struct ks0713 *self);
void ks0713_move_cursor(struct ks0713 *self, uint8_t x, uint8_t y);
void ks0713_write_pixel(struct ks0713 *self, uint8_t x, uint8_t y, ks0713_pixel_op_t op);
//void ks0713_putc(struct ks0713 *self, uint8_t ch, uint16_t flags);
//void ks0713_puts(const char *s, LCD_OP op, uint16_t flags);
void ks0713_draw_line(struct ks0713 *self, int8_t x1, int8_t y1, int8_t x2, int8_t y2, ks0713_pixel_op_t op);
void ks0713_draw_rect(struct ks0713 *self, int8_t x1, int8_t y1, int8_t x2, int8_t y2, ks0713_pixel_op_t op);

tty_dev_t ks0713_get_tty_interface(struct ks0713 *self); 

#endif // _LCD_H
