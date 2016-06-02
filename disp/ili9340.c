/*
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#include <arch/soc.h>

#include "ili9340.h"

#define CS_HI 		pio_set_pin(self->gpio, self->cs_pin)
#define CS_LO 		pio_clear_pin(self->gpio, self->cs_pin)
#define RST_HI 		pio_set_pin(self->gpio, self->rst_pin)
#define RST_LO 		pio_clear_pin(self->gpio, self->rst_pin)
#define DC_HI 		pio_set_pin(self->gpio, self->dc_pin)
#define DC_LO 		pio_clear_pin(self->gpio, self->dc_pin)

//#define spi_writereadbyte(ch) (serial_putc(self->sdev, (ch)), serial_getc(self->sdev))

/*
static struct ili9340 {
	uint16_t screen_width, screen_height; 
	int16_t cursor_x, cursor_y;
	int8_t char_width, char_height;
	uint16_t back_color, front_color;
	uint16_t scroll_start; 
} term;
*/
static void _wr_command(struct ili9340 *self, uint8_t c) {
	uint8_t buf[2] = {c, 0}; 
	
	DC_LO;
	CS_LO;

	spi_transfer(self->spi, buf, buf+1, 1);

	CS_HI; 
}


static uint8_t _wr_data(struct ili9340 *self, uint8_t c) {
	uint8_t buf[2] = {c, 0}; 
	DC_HI; 
	CS_LO; 
	uint8_t r = spi_transfer(self->spi, buf, buf+1, 1);
	CS_HI;
	return r; 
} 

static uint16_t _wr_data16(struct ili9340 *self, uint16_t c){
	uint8_t buf[4] = {c>>8, c & 0xff, 0, 0}; 
	DC_HI; 
	CS_LO; 

	spi_transfer(self->spi, buf, buf + 2, 2); 

	CS_HI;
	return (buf[2] << 8 | buf[3]); 
}
// Rather than a bazillion _wr_command(self, ) and _wr_data(self, ) calls, screen
// initialization commands and arguments are organized in these tables
// stored in PROGMEM.  The table may look bulky, but that's mostly the
// formatting -- storage-wise this is hundreds of bytes more compact
// than the equivalent code.  Companion function follows.
#define DELAY 0x80

void ili9340_init(struct ili9340 *self, spi_dev_t spi, pio_dev_t gpio, gpio_pin_t cs_pin, gpio_pin_t dc_pin, gpio_pin_t rst_pin) {
	self->spi = spi;
	self->gpio = gpio; 
	self->cs_pin = cs_pin;
	self->dc_pin = dc_pin;
	self->rst_pin = rst_pin;
	
	pio_configure_pin(gpio, cs_pin, GP_OUTPUT); 
	pio_configure_pin(gpio, rst_pin, GP_OUTPUT); 
	pio_configure_pin(gpio, dc_pin, GP_OUTPUT);
	
	RST_LO; 

	RST_HI; 
	delay_us(5000); 
	RST_LO; 
	delay_us(20000);
	RST_HI; 
	delay_us(150000L);

	_wr_command(self, 0xEF);
	_wr_data(self, 0x03);
	_wr_data(self, 0x80);
	_wr_data(self, 0x02);

	_wr_command(self, 0xCF);  
	_wr_data(self, 0x00); 
	_wr_data(self, 0XC1); 
	_wr_data(self, 0X30); 

	_wr_command(self, 0xED);  
	_wr_data(self, 0x64); 
	_wr_data(self, 0x03); 
	_wr_data(self, 0X12); 
	_wr_data(self, 0X81); 

	_wr_command(self, 0xE8);  
	_wr_data(self, 0x85); 
	_wr_data(self, 0x00); 
	_wr_data(self, 0x78); 

	_wr_command(self, 0xCB);  
	_wr_data(self, 0x39); 
	_wr_data(self, 0x2C); 
	_wr_data(self, 0x00); 
	_wr_data(self, 0x34); 
	_wr_data(self, 0x02); 

	_wr_command(self, 0xF7);  
	_wr_data(self, 0x20); 

	_wr_command(self, 0xEA);  
	_wr_data(self, 0x00); 
	_wr_data(self, 0x00); 

	_wr_command(self, ILI9340_PWCTR1);    //Power control 
	_wr_data(self, 0x23);   //VRH[5:0] 

	_wr_command(self, ILI9340_PWCTR2);    //Power control 
	_wr_data(self, 0x10);   //SAP[2:0];BT[3:0] 

	_wr_command(self, ILI9340_VMCTR1);    //VCM control 
	_wr_data(self, 0x3e); //
	_wr_data(self, 0x28); 

	_wr_command(self, ILI9340_VMCTR2);    //VCM control2 
	_wr_data(self, 0x86);  //--

	_wr_command(self, ILI9340_MADCTL);    // Memory Access Control 
	_wr_data(self, ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);

	_wr_command(self, ILI9340_PIXFMT);    
	_wr_data(self, 0x55); 

	_wr_command(self, ILI9340_FRMCTR1);    
	_wr_data(self, 0x00);  
	_wr_data(self, 0x18); 

	_wr_command(self, ILI9340_DFUNCTR);    // Display Function Control 
	_wr_data(self, 0x08); 
	_wr_data(self, 0x82);
	_wr_data(self, 0x27);  

	_wr_command(self, 0xF2);    // 3Gamma Function Disable 
	_wr_data(self, 0x00); 

	_wr_command(self, ILI9340_GAMMASET);    //Gamma curve selected 
	_wr_data(self, 0x01); 

	_wr_command(self, ILI9340_GMCTRP1);    //Set Gamma 
	_wr_data(self, 0x0F); 
	_wr_data(self, 0x31); 
	_wr_data(self, 0x2B); 
	_wr_data(self, 0x0C); 
	_wr_data(self, 0x0E); 
	_wr_data(self, 0x08); 
	_wr_data(self, 0x4E); 
	_wr_data(self, 0xF1); 
	_wr_data(self, 0x37); 
	_wr_data(self, 0x07); 
	_wr_data(self, 0x10); 
	_wr_data(self, 0x03); 
	_wr_data(self, 0x0E); 
	_wr_data(self, 0x09); 
	_wr_data(self, 0x00); 

	_wr_command(self, ILI9340_GMCTRN1);    //Set Gamma 
	_wr_data(self, 0x00); 
	_wr_data(self, 0x0E); 
	_wr_data(self, 0x14); 
	_wr_data(self, 0x03); 
	_wr_data(self, 0x11); 
	_wr_data(self, 0x07); 
	_wr_data(self, 0x31); 
	_wr_data(self, 0xC1); 
	_wr_data(self, 0x48); 
	_wr_data(self, 0x08); 
	_wr_data(self, 0x0F); 
	_wr_data(self, 0x0C); 
	_wr_data(self, 0x31); 
	_wr_data(self, 0x36); 
	_wr_data(self, 0x0F); 

	_wr_command(self, ILI9340_SLPOUT);    //Exit Sleep 
	delay_us(120000L); 		
	_wr_command(self, ILI9340_DISPON);    //Display on

	self->screen_width = ILI9340_TFTWIDTH;
	self->screen_height = ILI9340_TFTHEIGHT;
	self->char_height = 8;
	self->char_width = 6;
	self->back_color = 0x0000;
	self->front_color = 0xffff;
	self->cursor_x = self->cursor_y = 0;
	self->scroll_start = 0; 

	memset(self->buffer, 0xfa, sizeof(self->buffer)); 
}

/*
void ili9340_setScrollStart(struct ili9340 *self, uint16_t start){
  _wr_command(self, 0x37); // Vertical Scroll definition.
  _wr_data16(self, start);
  term.scroll_start = start; 
}*/

static void _ili9340_setAddrWindow(struct ili9340 *self, int16_t x0, int16_t y0, int16_t x1, int16_t y1){
	_wr_command(self, ILI9340_CASET); // Column addr set
	_wr_data(self, x0 >> 8);
	_wr_data(self, x0 & 0xFF);     // XSTART 
	_wr_data(self, x1 >> 8);
	_wr_data(self, x1 & 0xFF);     // XEND

	_wr_command(self, ILI9340_PASET); // Row addr set
	_wr_data(self, y0>>8);
	_wr_data(self, y0);     // YSTART
	_wr_data(self, y1>>8);
	_wr_data(self, y1);     // YEND
}

void ili9340_setScrollMargins(struct ili9340 *self, uint16_t top, uint16_t bottom) {
  // Did not pass in VSA as TFA+VSA=BFA must equal 320
	_wr_command(self, 0x33); // Vertical Scroll definition.
	_wr_data16(self, top);
	_wr_data16(self, 320-(top+bottom));
	_wr_data16(self, bottom); 
}

void ili9340_update(struct ili9340 *self){
	uint8_t buf[4]; 

	_ili9340_setAddrWindow(self, self->screen_width, self->screen_height, self->screen_width - 1, self->screen_height - 1);
	
	CS_LO;
 	DC_LO; 
	
	buf[0] = ILI9340_RAMWR;
	spi_transfer(self->spi, buf, buf + 1, 1); 
	//spi_writereadbyte(0);
	
	DC_HI;
	uint8_t *data = self->buffer; 

	for(size_t c = 0; c < sizeof(self->buffer); c++){
		uint16_t r = 0; 
		spi_transfer(self->spi, data + c, (uint8_t*)&r, 2);  
		data+=2; 
		//spi_writereadbyte(d >> 8);
		//spi_writereadbyte(d);
	}

	CS_HI; 
}


/*
static void ili9340_setAddrWindow_WR(struct ili9340 *self, int16_t x0, int16_t y0, int16_t x1,
 int16_t y1) {
  _ili9340_setAddrWindow(self, x0, y0, x1, y1); 
  _wr_command(self, ILI9340_RAMWR); // write to RAM
}
*/
/*
void ili9340_readRect(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data){
	_ili9340_setAddrWindow(self, x, y, x + w - 1, y + h - 1);
	//_wr_command(self, ILI9340_RAMRD); // read RAM
	CS_LO;
	DC_LO; 
	
	spi_writereadbyte(ILI9340_RAMRD);
	//spi_writereadbyte(0);

	DC_HI;

	spi_writereadbyte(0);
	
 	uint16_t cnt = w * h;
 	for(uint16_t c = 0; c < cnt; c++){
		// DAMN! this took a while to figure out :)
		// The data is one byte per color! NOT packed!
		data[c] = RGB16(spi_writereadbyte(0), spi_writereadbyte(0), spi_writereadbyte(0));
	}

	CS_HI; 
}

void ili9340_writeRect(struct ili9340 *self,uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *data){
  _ili9340_setAddrWindow(self, x, y, x + w - 1, y + h - 1);
	
	CS_LO;
 	DC_LO; 
	
	spi_writereadbyte(ILI9340_RAMWR);
	//spi_writereadbyte(0);
	
	DC_HI;
	
  uint16_t cnt = w * h;
  for(uint16_t c = 0; c < cnt; c++){
		uint16_t d = *data++;
		spi_writereadbyte(d >> 8);
		spi_writereadbyte(d);
	}

	CS_HI; 
}
*/
/*
static void ili9340_pushColor(struct ili9340 *self, uint16_t color) {
  DC_HI;
  CS_LO; 

  spi_writereadbyte(color >> 8);
  spi_writereadbyte(color);

	CS_HI; 
}*/
uint16_t ili9340_width(struct ili9340 *self){
	return self->screen_width;
}

uint16_t ili9340_height(struct ili9340 *self){
	return self->screen_height;
}

// fill a rectangle
/*void ili9340_fillRect(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t w, uint16_t h,
  uint16_t color) {
	//y = (y + term.scroll_start) % term.screen_height;
	
  // rudimentary clipping (drawChar w/big text requires this)
  //if((x >= t->screen_width) || (y >= t->screen_height)) return;
  if((x + w - 1) >= self->screen_width)  w = self->screen_width  - x;
  if((y + h - 1) >= self->screen_height) h = self->screen_height - y;

	ili9340_setAddrWindow_WR(self, 
		x, 					y,
		x + w - 1, 	y + h - 1);

	uint8_t hi = color >> 8, lo = color;

	DC_HI; 
	CS_LO; 
	
	for(y=h; y>0; y--) {
		for(x=w; x>0; x--) {
			spi_writereadbyte(hi);
			spi_writereadbyte(lo);
		}
	}
	CS_HI;
}

void ili9340_drawLine(struct ili9340 *self, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t col){
  int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1; 
  int err = (dx>dy ? dx : -dy)/2, e2;
	
	uint8_t hi = col >> 8, lo = col;
	
  for(;;){
		ili9340_setAddrWindow_WR(self, x0, y0, x0, y0);

		DC_HI; 
		CS_LO; 
		
		spi_writereadbyte(hi);
		spi_writereadbyte(lo);
		
		CS_HI;
		
    if (x0==x1 && y0==y1) break;
    e2 = err;
    if (e2 >-dx) { err -= dy; x0 += sx; }
    if (e2 < dy) { err += dx; y0 += sy; }
  }
  
}
*/
/*
THIS CODE BELONGS SOMEWHERE ELSE

void ili9340_setBackColor(struct ili9340 *self, uint16_t col){
	//uint8_t r, uint8_t g, uint8_t b
	struct ili9340 *t = &term;
	t->back_color = col; 
	//t->back_color = (uint16_t)r << 8 | (uint16_t)g << 4 | b; 
}

void ili9340_setFrontColor(uint16_t col){
	struct ili9340 *t = &term;
	t->front_color = col; 
	//t->front_color = (uint16_t)r << 8 | (uint16_t)g << 4 | b; 
}

void ili9340_drawChar(uint16_t x, uint16_t y, uint8_t ch){
	struct ili9340 *t = &term;
	
	ili9340_setAddrWindow_WR(x, y, x+t->char_width-1, y + t->char_height);

	DC_HI;
	CS_LO;

	// character glyph buffer
	char _buf[5]; 
	for(int j = 0; j < 5; j++){
		_buf[j] = pgm_read_byte(&font[ch * 5 + j]);
	}
	for(int b = 0; b < 8; b++){
		// draw 5 pixels for each column of the glyph
		for(int j = 0; j < 5; j++){
			uint16_t pix = t->back_color;
			if(_buf[j] & (1 << b))
				pix = t->front_color;
			spi_writereadbyte(pix >> 8);
			spi_writereadbyte(pix);
		}
		
		// draw one more separator pixel
		spi_writereadbyte(t->back_color >> 8);
		spi_writereadbyte(t->back_color);
	}
	CS_HI;
}

void ili9340_drawString(uint16_t x, uint16_t y, const char *text){
	//static char _buffer[128]; // buffer for 1 char
	//int len = strlen(text);
	struct ili9340 *t = &term;
	
	for(const char *_ch = text; *_ch; _ch++){
		//DDRD |= _BV(5);
		//PORTD |= _BV(5); 
		if(!*_ch) break;
		
		ili9340_drawChar(x, y, *_ch);
		x += t->char_width; 
		//PORTD &= ~_BV(5); 
	}

}

void ili9340_drawSprite(uint16_t x, uint16_t y, const uint8_t *sprite, const uint16_t *palette){

  ili9340_setAddrWindow_WR(x, y, x + 8 - 1, y + 8 - 1);

	DC_HI;
  CS_LO; 

  for(int c = 0; c < 64; c++){
		uint8_t idx = pgm_read_byte(sprite + c);
		uint16_t pix = pgm_read_word(palette + idx); 
		spi_writereadbyte(pix >> 8);
    spi_writereadbyte(pix);
  }
  
  CS_HI; 
}
*/
/*
void ili9340_drawFastHLine(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t w,
  uint16_t color) {
  // Rudimentary clipping
  
	//y = (y + term.scroll_start) % term.screen_height;
	
  if((x >= self->screen_width) || (y >= self->screen_height)) return;
  if((x+w-1) >= self->screen_width)  w = self->screen_width-x;
  
  ili9340_setAddrWindow_WR(self, x, y, x+w-1, y);

  uint8_t hi = color >> 8, lo = color;
  DC_HI;
  CS_LO; 
  while (w--) {
    spi_writereadbyte(hi);
    spi_writereadbyte(lo);
  }
  CS_HI; 
}


void ili9340_drawFastVLine(struct ili9340 *self, uint16_t x, uint16_t y, uint16_t h,
  uint16_t color) {
	
	ili9340_fillRect(self, (h > 0)?x:(x + h), y, 1, (h > 0)?(x + h):x, color); 
}

void ili9340_setRotation(struct ili9340 *self, uint8_t m) {
  _wr_command(self, ILI9340_MADCTL);
  int rotation = m % 4; // can't be higher than 3
  switch (rotation) {
   case 0:
     _wr_data(self, ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     self->screen_width  = ILI9340_TFTWIDTH;
     self->screen_height = ILI9340_TFTHEIGHT;
     break;
   case 1:
     _wr_data(self, ILI9340_MADCTL_MV | ILI9340_MADCTL_BGR);
     self->screen_width  = ILI9340_TFTHEIGHT;
     self->screen_height = ILI9340_TFTWIDTH;
     break;
  case 2:
    _wr_data(self, ILI9340_MADCTL_MY | ILI9340_MADCTL_BGR);
     self->screen_width  = ILI9340_TFTWIDTH;
     self->screen_height = ILI9340_TFTHEIGHT;
    break;
   case 3:
     _wr_data(self, ILI9340_MADCTL_MV | ILI9340_MADCTL_MY | ILI9340_MADCTL_MX | ILI9340_MADCTL_BGR);
     self->screen_width  = ILI9340_TFTHEIGHT;
     self->screen_height = ILI9340_TFTWIDTH;
     break;
  }
}
*/
