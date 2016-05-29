/*******
 * I2C asynchronous driver for ssd1306 series 128x64 point displays
 *
 * License: GPLv3
 * Author: Martin K. Schröder
 * Email: info@fortmax.se
 * Website: http://oskit.se
 */

#include <stdlib.h>
#include <arch/soc.h>
#include <errno.h>

//#include "lcd_font_5x7.h"

#include <string.h>

#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#define ABS(x) ((x >= 0) ? x : -x)

#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF

#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA

#define SSD1306_SETVCOMDETECT 0xDB

#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9

#define SSD1306_SETMULTIPLEX 0xA8

#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTPAGE 0xb0
#define SSD1306_SETSTARTLINE 0x40

#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COLUMNADDR 0x21
#define SSD1306_PAGEADDR   0x22

#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8

#define SSD1306_SEGREMAP 0xA0

#define SSD1306_CHARGEPUMP 0x8D

#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2

// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH  128
#define FRAGMENT_WIDTH 128
#define FRAGMENT_HEIGHT 8


#define TEXT_W (21)
#define TEXT_H (8)
#define TEXTSZ (TEXT_W * TEXT_H)
#define TEXTPACKETSZ (8)
#define TEXTCHAR_W (5)
#define PACKETSZ (TEXTPACKETSZ * 6 + 1) // 8 chars * 6 bytes each + 1 command byte
#define debug(msg) {} //uart_printf("SSD1306: %s\n", msg);

#define SSD1306_128_64

#if defined SSD1306_128_64 && defined SSD1306_128_32
#error "Only one SSD1306 display can be specified at once in SSD1306.h"
#endif
#if !defined SSD1306_128_64 && !defined SSD1306_128_32
#error "At least one SSD1306 display must be specified in SSD1306.h"
#endif
#if defined SSD1306_128_64
#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 64
#endif
#if defined SSD1306_128_32
#define SSD1306_LCDWIDTH 128
#define SSD1306_LCDHEIGHT 32
#endif
#define SSD1306_SETCONTRAST 0x81
#define SSD1306_DISPLAYALLON_RESUME 0xA4
#define SSD1306_DISPLAYALLON 0xA5
#define SSD1306_NORMALDISPLAY 0xA6
#define SSD1306_INVERTDISPLAY 0xA7
#define SSD1306_DISPLAYOFF 0xAE
#define SSD1306_DISPLAYON 0xAF
#define SSD1306_SETDISPLAYOFFSET 0xD3
#define SSD1306_SETCOMPINS 0xDA
#define SSD1306_SETVCOMDETECT 0xDB
#define SSD1306_SETDISPLAYCLOCKDIV 0xD5
#define SSD1306_SETPRECHARGE 0xD9
#define SSD1306_SETMULTIPLEX 0xA8
#define SSD1306_SETLOWCOLUMN 0x00
#define SSD1306_SETHIGHCOLUMN 0x10
#define SSD1306_SETSTARTLINE 0x40
#define SSD1306_MEMORYMODE 0x20
#define SSD1306_COMSCANINC 0xC0
#define SSD1306_COMSCANDEC 0xC8
#define SSD1306_SEGREMAP 0xA0
#define SSD1306_CHARGEPUMP 0x8D
#define SSD1306_EXTERNALVCC 0x1
#define SSD1306_SWITCHCAPVCC 0x2
// Scrolling #defines
#define SSD1306_ACTIVATE_SCROLL 0x2F
#define SSD1306_DEACTIVATE_SCROLL 0x2E
#define SSD1306_SET_VERTICAL_SCROLL_AREA 0xA3
#define SSD1306_RIGHT_HORIZONTAL_SCROLL 0x26
#define SSD1306_LEFT_HORIZONTAL_SCROLL 0x27
#define SSD1306_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29
#define SSD1306_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A

//#define SSD1306_I2C_ADDR 0x78
// normal i2c address on linux is 3c, shifted is 78
#define SSD1306_I2C_ADDR 0x3c
#define SSD1306_128_64

#define U8G_ESC_CS(x) 255, (0xd0 | ((x)&0x0f))
#define U8G_ESC_ADR(x) 255, (0xe0 | ((x)&0x0f))
#define U8G_ESC_RST(x) 255, (0xc0 | ((x)&0x0f))
#define U8G_ESC_END 255, 254

static const uint8_t cmd_init[] PROGMEM = {
	U8G_ESC_CS(0),             //disable chip
	U8G_ESC_ADR(0),           /* instruction mode */
	U8G_ESC_RST(1),           /* do reset low pulse with (1*16)+2 milliseconds */
	U8G_ESC_CS(1),             /* enable chip */
	
	0x0ae,				/* display off, sleep mode */
	0x0d5, 0x081,		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
	0x0a8, 0x03f,		/* multiplex ratio */
	0x0d3, 0x000,	0x00,	/* display offset */
	//0x040,				/* start line */
	0x08d, 0x14,		/* charge pump setting (p62): 0x014 enable, 0x010 disable */
	0x020, 0x00, // memory addr mode
	0x0a1,				/* segment remap a0/a1*/
	0x0a5, // display on
	0x0c8,				/* c0: scan dir normal, c8: reverse */
	0x0da, 0x012,		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
	0x081, 0x09f,		/* set contrast control */
	0x0d9, 0x011,		/* pre-charge period */
	0x0db, 0x020,		/* vcomh deselect level */
	0x021, 0x00, 0x7f, // (7d for 125 col screen!) column addressing mode
	0x022, 0x00, 0x07,		/* page addressing mode WRONG: 3 byte cmd! */
	0x0a4,				/* output ram to display */
	0x0a6,				/* none inverted normal display mode */
	0x0af,				/* display on */

	U8G_ESC_CS(0),             /* disable chip */
	U8G_ESC_END                /* end of sequence */
};

static const unsigned char cmd_home[] PROGMEM = {
	U8G_ESC_ADR(0),           // instruction mode 
	U8G_ESC_CS(1),             // enable chip 
	0x000, // | (col & 0x0f),		// set lower 4 bit of the col adr to 0 
	0x010, // | ((col >> 4) & 0x0f),		// set higher 4 bit of the col adr to 0 
	0x0b0, // | row & 0x0f,  	// page address
	U8G_ESC_END,                // end of sequence 
};
#include <kernel/mt.h>
#include "display.h"

struct ssd1306 {
	struct display_device device; 
	struct i2c_adapter *adapter; 
	uint8_t addr; 
	char buffer[1 * 128 + 1]; 
	mutex_t lock; 
};

static int ssd1306_write_cmd(struct ssd1306 *self, uint8_t cmd){
	char data[] = { 0, cmd }; 
	if(i2c_write(self->adapter, self->addr, data, 2) < 0) return -EIO; 
	return 0; 
}

#if 0
static int ssd1306_write_data(struct ssd1306 *self, uint8_t data){
	char buf[] = { 0x40, data }; 
	if(i2c_write(self->adapter, self->addr, buf, 2) < 0) return -EIO; 
	return 0; 
}
#endif

static int ssd1306_write_array(struct ssd1306 *self, char *buffer, size_t size){
	if(i2c_write(self->adapter, self->addr, buffer, size) < 0) return -EIO; 
	return 0; 
}

/*
static void ssd1306_data_buffer(struct ssd1306 *self, uint8_t *data, uint16_t size){
	i2c_start_write(self->i2c, DISPLAY_ADDRESS, data, size);
	i2c_stop(self->i2c); 
}
*/

#if 0
static void ssd1306_clear(struct ssd1306 *dev){
	//ssd1306_gotoChar(dev, 0, 0); ssd1306_write_cmd(dev, SSD1306_COLUMNADDR);
	ssd1306_write_cmd(dev, 0);   // Column start address (0 = reset)
	ssd1306_write_cmd(dev, 127); // Column end address (127 = reset)

	ssd1306_write_cmd(dev, SSD1306_PAGEADDR);
	ssd1306_write_cmd(dev, 0); // Page start address (0 = reset)
	ssd1306_write_cmd(dev, (SSD1306_LCDHEIGHT == 64) ? 7 : 3); // Page end address
}
#endif
/*
int16_t ssd1306_puts(ssd1306_t *dev, const char *str, uint8_t col){
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

static void ssd1306_set_region(ssd1306_t *dev, uint16_t x0, uint16_t y0, uint16_t w, uint16_t h){
	ssd1306_write_cmd(dev, SSD1306_SETLOWCOLUMN | (x0 & 0x0f)); 
	ssd1306_write_cmd(dev, SSD1306_SETHIGHCOLUMN | ((x0 >> 4) & 0x0f)); 
	ssd1306_write_cmd(dev, SSD1306_SETSTARTPAGE | ((y0 >> 3) & 0x07)); 
	
	ssd1306_write_cmd(dev, SSD1306_COLUMNADDR);
	ssd1306_write_cmd(dev, x0);   // Column start address (0 = reset)
	ssd1306_write_cmd(dev, x0 + w + 1); // Column end address (127 = reset)

	ssd1306_write_cmd(dev, SSD1306_PAGEADDR);
	ssd1306_write_cmd(dev, y0 >> 3); // Page start address (0 = reset)
	ssd1306_write_cmd(dev, ((y0 + h) >> 3) + 1); // Page end address
  
}

void ssd1306_draw(ssd1306_t *self, uint16_t x, uint16_t y, fb_image_t img){
	ssd1306_set_region(self, x, y, img.w, img.h); 
	
	// writing needs to be done in columns of 8 pixels
	uint8_t byte = 0; 
	size_t idx = 0;
	//size_t total_size = (img.w * img.h) >> 3; 
	uint8_t buffer[8]; 
	
	for(unsigned top = 0; top < img.h; top++){
		for(unsigned left = 0; left < img.w; left++){
			for(unsigned col = 0; (col < 8 && (left + col) < img.w); col++){
				for(unsigned row = 0; (row < 8 && (top + row) < img.h); row++){
					if(img.data[left + col + ((top + row) * img.w) + 1] & _BV(7 - col)){
						byte |= _BV(7 - col); 
					} 
				}
				buffer[idx++] = byte;
				byte = 0; 
			}
			ssd1306_data_buffer(self, buffer, idx); 
			idx = 0; 
		}
	}
}

void ssd1306_fill(ssd1306_t *self, fb_color_t col, size_t len){
	uint8_t color = 0; 
	uint8_t buf[32];
	uint8_t bi = 0; 
	
	if(col.r || col.g || col.b || col.a) color = 1; else color = 0; 
	uint8_t byte = 0; 
	
	for(size_t c = 0; c < len; c++){
		if(bi == sizeof(buf)){
			ssd1306_data_buffer(self, buf, bi); 
			bi = 0; 
		} 
		
		byte |= color; 
		if((c & 0x07) == 0x07){
			buf[bi++] = (color)?0xf0:0; 
			byte = 0; 
		} else {
			byte <<= 1; 
		}
	}
	ssd1306_data_buffer(self, buf, bi);  
}

int16_t ssd1306_printf(ssd1306_t *dev, uint8_t col, const char *fmt, ...){
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	char buf[64]; 
	
	n = vsnprintf(buf, 64, fmt, vl);
	
	va_end(vl);

	ssd1306_puts(dev, buf, col); 
	
	return n; 
}*/

#if 0
static int ssd1306_set_address(struct ssd1306 *self, int col, int row){
	uint8_t home[] = {
		U8G_ESC_ADR(0),           // instruction mode 
		U8G_ESC_CS(1),             // enable chip 
		0x00 | (col & 0x0f),		// set lower 4 bit of the col adr to 0 
		0x10 | ((col >> 4) & 0x0f),		// set higher 4 bit of the col adr to 0 
		0xb0 | (row & 0x07),  	// page address
		
		0x021, col, 0x7f, // (7d for 125 col screen!) column addressing mode
		0x022, row, 0x07,		
	
		U8G_ESC_END,                // end of sequence 
	}; 
	for(unsigned c = 0; c < sizeof(home); c++){
		if(ssd1306_write_cmd(self, home[c]) < 0) return -EIO; 
	}
	return 0; 
}
static int ssd1306_read(struct ssd1306 *self, char *out, size_t size){
	if(i2c_read(self->adapter, self->addr, out, size) < 0) return -EIO; 
	return 0; 
}
#endif

#include <serial/serial.h>
static int ssd1306_draw_pixel(struct display_device *dev, uint16_t x, uint16_t y, uint32_t color){
	struct ssd1306 *self = container_of(dev, struct ssd1306, device); 
	char *buf = &self->buffer[1]; 
	int page = y >> 3; 
	uint8_t bit = y & 0x7; 

	ssd1306_write_cmd(self, SSD1306_SETLOWCOLUMN); 
	ssd1306_write_cmd(self, SSD1306_SETHIGHCOLUMN); 
	ssd1306_write_cmd(self, SSD1306_SETSTARTPAGE); 

	ssd1306_write_cmd(self, SSD1306_PAGEADDR);
	ssd1306_write_cmd(self, 0); // Page start address (0 = reset)
	ssd1306_write_cmd(self, 7); // Page end address   

	ssd1306_write_cmd(self, SSD1306_COLUMNADDR);
	ssd1306_write_cmd(self, 0);   // Column start address (0 = reset)
	ssd1306_write_cmd(self, 127); // Column end address (127 = reset)

	if(color) *(buf+x) |= _BV(bit); 
	else *(buf+x) &= ~_BV(bit); 

	self->buffer[0] = 0x40; 
	
	ssd1306_write_array(self, self->buffer, sizeof(self->buffer)); 

	return 0; 
}

static int ssd1306_init_display(struct ssd1306 *self){
	for(unsigned c = 0; c < sizeof(cmd_init); c++){
		if(ssd1306_write_cmd(self, pgm_read_byte(&cmd_init[c])) < 0) return -EIO; 
	}
	return 0; 
}

static struct display_device_ops ssd1306_ops = {
	.draw_pixel = ssd1306_draw_pixel
}; 

struct display_device *ssd1306_new(struct i2c_adapter *adapter, uint8_t address){
	if(!i2c_device_exists(adapter, address)) return NULL; 
	struct ssd1306 *self = kzmalloc(sizeof(struct ssd1306)); 
	if(!self) return 0; 
	self->adapter = adapter;
	self->addr = address;
	self->device.ops = &ssd1306_ops; 
	mutex_init(&self->lock); 
	ssd1306_init_display(self); 
	return &self->device; 
}

