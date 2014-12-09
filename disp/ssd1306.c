/*******
 * I2C asynchronous driver for ssd1306 series 128x64 point displays
 *
 * License: GPLv3
 * Author: Martin K. Schröder
 * Email: info@fortmax.se
 * Website: http://oskit.se
 */

#ifdef __AVR__
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#else
#define PROGMEM
#define pgm_read_byte(a) (*a)
#endif

#include <stdlib.h>
#include <arch/soc.h>

#include "ssd1306.h"
#include "ssd1306_priv.h"

#include <string.h>

#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define ABS(x) ((x >= 0) ? x : -x)

#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH  128
#define FRAGMENT_WIDTH 128
#define FRAGMENT_HEIGHT 8

#define DISPLAY_ADDRESS 0x78

#define TEXT_W (21)
#define TEXT_H (8)
#define TEXTSZ (TEXT_W * TEXT_H)
#define TEXTPACKETSZ (8)
#define TEXTCHAR_W (5)
#define PACKETSZ (TEXTPACKETSZ * 6 + 1) // 8 chars * 6 bytes each + 1 command byte
#define debug(msg) {} //uart_printf("SSD1306: %s\n", msg);

void ssd1306_command(uint8_t cmd){
	i2c_start_wait(DISPLAY_ADDRESS | I2C_WRITE);
	i2c_write(0x00);
	i2c_write(cmd); 
	i2c_stop();
}

void ssd1306_init(ssd1306_device_t *dev){
	i2c_init(); 
	
	for(int c = 0; c < sizeof(cmd_init); c++){
		ssd1306_command(pgm_read_byte(&cmd_init[c])); 
	}
}

void ssd1306_gotoChar(ssd1306_device_t *dev, uint8_t x, uint8_t y){
	uint8_t col = x * 6; 
	uint8_t row = y; 
	uint8_t home[] = {
		U8G_ESC_ADR(0),           // instruction mode 
		U8G_ESC_CS(1),             // enable chip 
		0x00 | (col & 0x0f),		// set lower 4 bit of the col adr to 0 
		0x10 | ((col >> 4) & 0x0f),		// set higher 4 bit of the col adr to 0 
		0xb0 | (row & 0x07),  	// page address
		
		0x021, col, 0x7f, // (7d for 125 col screen!) column addressing mode
		0x022, row, 0x07,		/* page addressing mode WRONG: 3 byte cmd! */
	
		U8G_ESC_END,                // end of sequence 
	}; 
	for(int c = 0; c < sizeof(home); c++){
		ssd1306_command(home[c]); 
	}
}

void ssd1306_clear(ssd1306_device_t *dev){
	ssd1306_gotoChar(dev, 0, 0); 
	i2c_start_wait(DISPLAY_ADDRESS | I2C_WRITE);
	i2c_write(0x40);
	for(int c = 0; c < (127 * 8); c++){
		i2c_write(0); 
	}
	i2c_stop();
}

int16_t ssd1306_puts(ssd1306_device_t *dev, const char *str, uint8_t col){
	i2c_start_wait(DISPLAY_ADDRESS | I2C_WRITE);
	i2c_write(0x40);
	for(int c = 0; c < strlen(str); c++){
		uint16_t start = str[c] * 5; 
		for(int j = 0; j < 5; j++){
			uint8_t ch = pgm_read_byte(&font[start + j]); 
			if(!col) ch = ~ch; 
			ch &= ~0x80; 
			i2c_write(ch); 
		}
		i2c_write((col)?0:0x7f); // insert spacing
	}
	i2c_stop();
	return 1; 
}
	
int16_t ssd1306_printf(ssd1306_device_t *dev, uint8_t col, const char *fmt, ...){
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	char buf[64]; 
	
	n = vsnprintf(buf, 64, fmt, vl);
	 
	va_end(vl);

	ssd1306_puts(dev, buf, col); 
	
	return n; 
}

