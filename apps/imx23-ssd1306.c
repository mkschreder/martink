#include <kernel.h>
#include <disp/ssd1306.h>
#include <arch/linux/i2c_device.h>
#include <kernel/dev/framebuffer.h>

struct app {
	struct libk_device device; 
	i2c_dev_t bus; 
	fb_dev_t fb; 
}; 

DEVICE_CLOCK(app_clock){
	struct libk_device *device = DEVICE_CLOCK_GET_DEVICE(); 
	struct app *self = container_of(device, struct app, device); 
	const char *buffer = "---:::---:::---:::---:::---"; 
	ASYNC_BEGIN();
	while(1){
		DEBUG("ssd: start\n"); 
		AWAIT(fbdev_open(self->fb)); 
		fbdev_seek(self->fb, 0, 0); 
		fbdev_write(self->fb, (const uint8_t*)buffer, 10); 
		fbdev_close(self->fb); 
		DEBUG("ssd: done!\n"); 
		ASYNC_YIELD(); 
	}
	ASYNC_END(0); 
}

static void __init app_init(void){
	static struct app app;
	static struct linux_i2c_device i2c_dev; 
	static struct ssd1306 ssd1306; 
	linux_i2c_device_init(&i2c_dev);
	ssd1306_init(&ssd1306, linux_i2c_device_get_interface(&i2c_dev)); 
	app.fb = ssd1306_get_framebuffer_interface(&ssd1306); 
	libk_register_device(&app.device, DEVICE_CLOCK_PTR(app_clock), "imx23-ssd1306"); 	
}
