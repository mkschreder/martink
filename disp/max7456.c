/* 
Copyright 2016 Martin Schröder <mkschreder.uk@gmail.com>
License: GPLv3. 
This file is part of martink project
*/

#include <arch/soc.h>
#include "max7456.h"
#include <kernel/mt.h>

//MAX7456 register addresses
#define MAX7456_REG_VM0		0x00
#define MAX7456_REG_VM1		0x01
#define MAX7456_REG_HOS		0x02
#define MAX7456_REG_VOS		0x03
#define MAX7456_REG_DMM		0x04
#define MAX7456_REG_DMAH	0x05
#define MAX7456_REG_DMAL	0x06
#define MAX7456_REG_DMDI	0x07
#define MAX7456_REG_CMM		0x08
#define MAX7456_REG_CMAH	0x09
#define MAX7456_REG_CMAL	0x0a
#define MAX7456_REG_CMDI	0x0b

#define MAX7456_REG_ROW_BASE 0x10

// read only registers
#define MAX7456_REG_STAT	0xa0
#define MAX7456_REG_CMD0	0xc1

// video mode 0 flags
#define VM0_VIDEO_BUFFER_DISABLE 	(1 << 0)
#define VM0_SOFT_RESET			(1 << 1)
#define VM0_SYNC_NEXT_VSYNC		(1 << 2)
#define VM0_OSD_ENABLE			(1 << 3)
#define VM0_SYNC_MODE_AUTO		0
#define VM0_SYNC_MODE_INTERNAL	(3 << 4)
#define VM0_SYNC_MODE_EXTERNAL	(2 << 4)
#define VM0_VIDEO_MODE_PAL		(1 << 6)
#define VM0_VIDEO_MODE_NTSC		0
	
// video mode 1 flags
#define VM1_BLINK_DUTY_CYCLE_50_50 0x00
#define VM1_BLINK_DUTY_CYCLE_33_66 0x01
#define VM1_BLINK_DUTY_CYCLE_25_75 0x02
#define VM1_BLINK_DUTY_CYCLE_75_25 0x03
#define VM1_BLINK_TIME_0 0x00
#define VM1_BLINK_TIME_1 0x04
#define VM1_BLINK_TIME_2 0x08
#define VM1_BLINK_TIME_3 0x0C

#define DMM_CLEAR_DISPLAY 0x04
#define MAX7456_DMDI_ESCAPE_CHAR 0xff

#define MAX7456_WLEVEL_80 0x03
#define MAX7456_WLEVEL_90 0x02
#define MAX7456_WLEVEL_100 0x01
#define MAX7456_WLEVEL_120 0x00

#define MAX7456_ROWS_COUNT 16
#define MAX7456_COLS_COUNT 30

#define max7456_begin(self) gpio_clear_pin((self)->gpio, (self)->cs_pin); 
#define max7456_end(self) gpio_set_pin((self)->gpio, (self)->cs_pin); 

static void max7456_write_reg(struct max7456 *self, uint8_t reg, uint8_t value){
	char data[2] = {reg, value}; 
	max7456_begin(self); 
	spi_transfer(self->spi, data, 2); 
	max7456_end(self); 
}

static void max7456_set_white_level(struct max7456 *self, uint8_t row_white_level){
	for(int c = 0; c < MAX7456_ROWS_COUNT; c++){
		max7456_write_reg(self, MAX7456_REG_ROW_BASE + c, row_white_level); 
	}
}

static void _tty_put(struct tty_device *dev, uint8_t ch, tty_color_t fg, tty_color_t bg){
	struct max7456 *self = container_of(dev, struct max7456, device); 
	max7456_write_char_at(self, self->cursor_x, self->cursor_y, ch); 
}

/// move cursor to a new position on the screen. 
static void _tty_move_cursor(struct tty_device *dev, uint16_t x, uint16_t y){
	struct max7456 *self = container_of(dev, struct max7456, device); 
	self->cursor_x = x; 
	self->cursor_y = y; 
}

/// get the size of the screen
static void _tty_get_size(struct tty_device *dev, uint16_t *w, uint16_t *h){
	// PAL
	*w = 30; 
	*h = 16; 
}

/// clear the screen
static void _tty_clear(tty_dev_t dev){
	uint16_t w, h; 
	struct max7456 *self = container_of(dev, struct max7456, device); 
	_tty_get_size(dev, &w, &h); 
	for(unsigned int x = 0; x < w; x++){
		for(unsigned int y = 0; y < h; y++){
			max7456_write_char_at(self, x, y, ' '); 
		}
	}
}

static struct tty_device_ops max7456_ops = {
	.put = _tty_put, 
	.move_cursor = _tty_move_cursor, 
	.get_size = _tty_get_size, 
	.clear = _tty_clear
}; 

void max7456_init(struct max7456 *self, struct spi_adapter *spi, struct gpio_adapter *gpio, gpio_pin_t cs_pin){
	self->spi = spi; 
	self->gpio = gpio; 
	self->cs_pin = cs_pin; 

	gpio_configure_pin(gpio, cs_pin, GP_OUTPUT); 

	max7456_write_reg(self, MAX7456_REG_VM0, VM0_VIDEO_MODE_PAL | VM0_SOFT_RESET); 	

	msleep(500); 

	max7456_write_reg(self, MAX7456_REG_HOS, 0x10); 
	max7456_write_reg(self, MAX7456_REG_VOS, 0x10);
	
	max7456_write_reg(self, MAX7456_REG_VM0, VM0_VIDEO_MODE_PAL | VM0_SYNC_NEXT_VSYNC | VM0_OSD_ENABLE); 

	max7456_set_white_level(self, MAX7456_WLEVEL_90); 	

	// init tty device
	self->device.ops = &max7456_ops; 
}

void max7456_write_char_at(struct max7456 *self, uint8_t x, uint8_t y, uint8_t ch){
	uint16_t addr = y * MAX7456_COLS_COUNT + x; 

	max7456_write_reg(self, MAX7456_REG_DMAH, addr >> 8); 
	max7456_write_reg(self, MAX7456_REG_DMAL, addr & 0xff); 
	max7456_write_reg(self, MAX7456_REG_DMDI, ch); 
}

struct tty_device* max7456_to_tty_device(struct max7456 *self){
	return &self->device; 
}
