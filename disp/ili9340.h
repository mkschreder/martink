/**
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#pragma once

#define ILI9340_TFTWIDTH  240
#define ILI9340_TFTHEIGHT 320

#define ILI9340_NOP     0x00
#define ILI9340_SWRESET 0x01
#define ILI9340_RDDID   0x04
#define ILI9340_RDDST   0x09

#define ILI9340_SLPIN   0x10
#define ILI9340_SLPOUT  0x11
#define ILI9340_PTLON   0x12
#define ILI9340_NORON   0x13

#define ILI9340_RDMODE  0x0A
#define ILI9340_RDMADCTL  0x0B
#define ILI9340_RDPIXFMT  0x0C
#define ILI9340_RDIMGFMT  0x0A
#define ILI9340_RDSELFDIAG  0x0F

#define ILI9340_INVOFF  0x20
#define ILI9340_INVON   0x21
#define ILI9340_GAMMASET 0x26
#define ILI9340_DISPOFF 0x28
#define ILI9340_DISPON  0x29

#define ILI9340_CASET   0x2A
#define ILI9340_PASET   0x2B
#define ILI9340_RAMWR   0x2C
#define ILI9340_RAMRD   0x2E

#define ILI9340_PTLAR   0x30
#define ILI9340_MADCTL  0x36


#define ILI9340_MADCTL_MY  0x80
#define ILI9340_MADCTL_MX  0x40
#define ILI9340_MADCTL_MV  0x20
#define ILI9340_MADCTL_ML  0x10
#define ILI9340_MADCTL_RGB 0x00
#define ILI9340_MADCTL_BGR 0x08
#define ILI9340_MADCTL_MH  0x04

#define ILI9340_PIXFMT  0x3A

#define ILI9340_FRMCTR1 0xB1
#define ILI9340_FRMCTR2 0xB2
#define ILI9340_FRMCTR3 0xB3
#define ILI9340_INVCTR  0xB4
#define ILI9340_DFUNCTR 0xB6

#define ILI9340_PWCTR1  0xC0
#define ILI9340_PWCTR2  0xC1
#define ILI9340_PWCTR3  0xC2
#define ILI9340_PWCTR4  0xC3
#define ILI9340_PWCTR5  0xC4
#define ILI9340_VMCTR1  0xC5
#define ILI9340_VMCTR2  0xC7

#define ILI9340_RDID1   0xDA
#define ILI9340_RDID2   0xDB
#define ILI9340_RDID3   0xDC
#define ILI9340_RDID4   0xDD

#define ILI9340_GMCTRP1 0xE0
#define ILI9340_GMCTRN1 0xE1
/*
#define ILI9340_PWCTR6  0xFC

*/


// Color definitions
#define	ILI9340_BLACK   0x0000
#define	ILI9340_BLUE    0x001F // 5 bit
#define	ILI9340_RED     0xF800 // 5 bit
#define	ILI9340_GREEN   0x07E0 // 6 bit
#define ILI9340_CYAN    0x07FF
#define ILI9340_MAGENTA 0xF81F
#define ILI9340_YELLOW  0xFFE0  
#define ILI9340_WHITE   0xFFFF


#define RGB16(R, G, B) (((uint16_t)((uint16_t)(R) >> 3) << 11) | ((uint16_t)(((uint16_t)G) >> 2) << 5) | ((uint16_t)(((uint16_t)B) >> 3)))
#define RGB16_SPLIT(COL, R, G, B) {R = ((COL & ILI9340_RED) >> 8); G = ((COL & ILI9340_GREEN) >> 5) << 2; B = (COL & ILI9340_BLUE) << 3;}


#ifdef __cplusplus
extern "C" {
#endif

struct ili9340 {
	serial_dev_t sdev;
	gpio_pin_t cs_pin, rst_pin, dc_pin;

	uint16_t screen_width, screen_height; 
	int16_t cursor_x, cursor_y;
	int8_t char_width, char_height;
	uint16_t back_color, front_color;
	uint16_t scroll_start; 
};

void ili9340_init(struct ili9340 *self, serial_dev_t spi, gpio_pin_t cs_pin, gpio_pin_t dc_pin, gpio_pin_t rst_pin);
void ili9340_drawFastVLine(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t h,
 uint16_t color);
void ili9340_drawFastHLine(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t h,
 uint16_t color);
void ili9340_drawLine(struct ili9340 *self, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, 
	uint16_t col); 
	
void ili9340_setRotation(struct ili9340 *self, uint8_t m) ;

void ili9340_drawString(struct ili9340 *self, uint16_t x, uint16_t y, const char *text);
void ili9340_drawChar(struct ili9340 *self, uint16_t x, uint16_t y, uint8_t c);
void ili9340_drawSprite(struct ili9340 *self, uint16_t x, uint16_t y, const uint8_t *sprite, const uint16_t *palette);

void ili9340_setBackColor(struct ili9340 *self, uint16_t col); 
void ili9340_setFrontColor(struct ili9340 *self, uint16_t col);
void ili9340_fillRect(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
  uint16_t color);

void ili9340_readRect(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);
void ili9340_writeRect(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data);

void ili9340_setScrollStart(struct ili9340 *self, uint16_t start); 
void ili9340_setScrollMargins(struct ili9340 *self, uint16_t top, uint16_t bottom);

uint16_t ili9340_width(struct ili9340 *self);
uint16_t ili9340_height(struct ili9340 *self);

#ifdef __cplusplus
}
#endif
