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

typedef struct fb_device **fb_dev_t; 

struct fb_device {
	/// open the device or return error if it is already open
	int (*open)(fb_dev_t dev); 
	/// close the device and release it for others to use
	int (*close)(fb_dev_t dev); 
	/// return the size of the pixel buffer of this device
	void (*get_size)(fb_dev_t dev, uint16_t *width, uint16_t *height);
	/// seek to an x,y position
	void (*seek)(fb_dev_t dev, uint16_t x, uint16_t y); 
	/// write to the framebuffer at position and increment position 
	int (*write)(fb_dev_t dev, const uint8_t *data, size_t count);
	/// read framebuffer 
	int (*read)(fb_dev_t dev, uint8_t *data, size_t count); 
}; 

#define fbdev_get_size(dev, w, h) (*dev)->get_size(dev, w, h)
#define fbdev_write(dev, data, len) (*dev)->write(dev, data, len)
#define fbdev_read(dev, data, len) (*dev)->read(dev, data, len)
#define fbdev_seek(dev, x, y) (*dev)->seek(dev, x, y)
#define fbdev_open(dev) (*dev)->open(dev)
#define fbdev_close(dev) (*dev)->close(dev)
