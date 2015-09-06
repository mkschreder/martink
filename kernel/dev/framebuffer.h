/*
 Generic framebuffer device interface. Currently oriented around providing 
 access to an array of pixels. But can be later extended to support larger
 blits and image transfers directly to the buffer. 
*/

#pragma once

/*
typedef enum {
	FB_PIXEL_FORMAT_1BIT, 
	FB_PIXEL_FORMAT_RGB8,
	FB_PIXEL_FORMAT_RGB16
} fb_pixel_type_t; 

typedef struct fb_image {
	uint16_t w; 
	uint16_t h;
	uint8_t *data; 
	fb_pixel_type_t format; 
} fb_image_t; 

typedef struct {
	uint8_t r, g, b, a; 
} fb_color_t; 

#define RGBA(r, g, b, a) (fb_color_t){r, g, b, a}
*/

typedef struct fbuf_device **fbuf_dev_t; 

struct fbuf_device {
	/// return the size of the pixel buffer of this device
	void (*get_size)(fbuf_dev_t dev, uint16_t *width, uint16_t *height);
	/// set a pixel in the framebuffer. Should check bounds! 
	void (*set_pixel)(fbuf_dev_t dev, uint16_t x, uint16_t y, uint16_t color); 
	/// clear the framebuffer (usually means all bits are zeroed)
	void (*clear)(fbuf_dev_t dev); 
}; 

#define fbuf_get_size(dev, w, h) (*dev)->get_size(dev, w, h); 
#define fbuf_set_pixel(dev, x, y, color) (*dev)->set_pixel(dev, x, y, color)
#define fbuf_clear(dev) (*dev)->clear(dev)
