#pragma once

#include <kernel/dev/framebuffer.h>
#include <kernel/cbuf.h>
#include <kernel.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SSD1306_I2C_ADDR 0x78
#define SSD1306_128_64
// #define SSD1306_128_32

typedef struct ssd1306 {
	struct libk_device device; 
	i2c_dev_t i2c; 
	uint8_t r_row_start, r_row_end;
	// private variables 
	uint8_t cmd_buffer[128*64*2]; 
	struct cbuf buffer; 
	uint8_t i2c_buf[2];
	uint8_t lock; 
	struct fb_device *api; 
} ssd1306_t;

void ssd1306_init(ssd1306_t *dev, i2c_dev_t i2c);
//void ssd1306_set_region(ssd1306_t *dev, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1); 
//void ssd1306_fill(ssd1306_t *dev, fb_color_t color, size_t len); 
//void ssd1306_draw(ssd1306_t *dev, uint16_t x, uint16_t y, fb_image_t img); 
//void ssd1306_gotoChar(ssd1306_t *dev, uint8_t x, uint8_t y); 
//void ssd1306_clear(ssd1306_t *dev); 
//int16_t ssd1306_puts(ssd1306_t *dev, const char *str, uint8_t col);
//int16_t ssd1306_printf(ssd1306_t *dev, uint8_t col, const char *str, ...); 
//void ssd1306_reset(ssd1306_t *dev);
fb_dev_t ssd1306_get_framebuffer_interface(struct ssd1306 *self); 

#ifdef __cplusplus
}
#endif
