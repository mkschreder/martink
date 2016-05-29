#pragma once

#include <kernel/types.h>
#include <i2c/i2c.h>

struct display_device; 
struct display_device_ops {
	int (*draw_pixel)(struct display_device *dev, uint16_t x, uint16_t y, uint32_t color); 
}; 

struct display_device {
	struct display_device_ops *ops; 
}; 

#define display_draw_pixel(dev, x, y, col) (dev)->ops->draw_pixel(dev, x, y, col)

struct display_device *ssd1306_new(struct i2c_adapter *adapter, uint8_t addr); 
