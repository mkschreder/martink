/*
Parallel LCD library for use with parallel interfaces

copyright (c)
* Martin Schröder 2014
* Davide Gironi, 2013

Released under GPLv3.
Please refer to LICENSE file for licensing information.

References:
  + based on lcd library by Peter Fleury
    http://jump.to/fleury
*/


#ifndef LCD_H
#define LCD_H

#include <inttypes.h>

#include <arch/interface.h>

struct lcd {
	pio_dev_t port; 
	uint8_t dataport; 
};

extern void lcd_init(struct lcd *self, pio_dev_t port, uint8_t dispAttr);
extern void lcd_clrscr(struct lcd *self);
extern void lcd_home(struct lcd *self);
extern void lcd_gotoxy(struct lcd *self, uint8_t x, uint8_t y);
extern void lcd_led(struct lcd *self, uint8_t onoff);
extern void lcd_putc(struct lcd *self, char c);
extern void lcd_puts(struct lcd *self, const char *s);
extern void lcd_puts_p(struct lcd *self, const char *progmem_s);
extern void lcd_command(struct lcd *self, uint8_t cmd);
extern void lcd_data(struct lcd *self, uint8_t data);

/*@}*/
#endif //LCD_H
