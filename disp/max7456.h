/* 
Copyright 2016 Martin Schröder <mkschreder.uk@gmail.com>
License: GPLv3. 
This file is part of martink project
*/

#pragma once

#include <arch/soc.h>

#include <spi/spi.h>
#include <gpio/gpio.h>
#include <tty/tty.h>

struct max7456 {
	struct spi_adapter *spi;
	struct gpio_adapter *gpio; 
	gpio_pin_t cs_pin;
	struct tty_device device; 
	uint8_t cursor_x, cursor_y; 
}; 

void max7456_init(struct max7456 *self, struct spi_adapter *spi, struct gpio_adapter *gpio, gpio_pin_t cs_pin); 
void max7456_write_char_at(struct max7456 *self, uint8_t x, uint8_t y, uint8_t ch); 

struct tty_device* max7456_to_tty_device(struct max7456 *self); 
