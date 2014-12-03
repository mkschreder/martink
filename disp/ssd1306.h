#pragma once

#define SSD1306_128_64
// #define SSD1306_128_32
/*=========================================================================*/
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

typedef struct ssd1306_device {
	
} ssd1306_device_t;

void ssd1306_init(ssd1306_device_t *dev);
void ssd1306_gotoChar(ssd1306_device_t *dev, uint8_t x, uint8_t y); 
void ssd1306_clear(ssd1306_device_t *dev); 
int16_t ssd1306_puts(ssd1306_device_t *dev, const char *str, uint8_t col);
int16_t ssd1306_printf(ssd1306_device_t *dev, uint8_t col, const char *str, ...); 
void ssd1306_reset(ssd1306_device_t *dev);

/*
int8_t ssd1306_puts(handle_t dev, const char *str, async_callback_t callback, void *ptr);
int8_t ssd1306_putraw(handle_t dev, const uint8_t *data, uint8_t size, async_callback_t callback, void *ptr); 
int8_t 	ssd1306_init(handle_t dev, async_callback_t callback, void *ptr); 
int8_t ssd1306_filltest(handle_t dev, async_callback_t callback, void *ptr); 
void ssd1306_seek(handle_t h, uint16_t addr); 
*/
