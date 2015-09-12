#include <kernel.h>
#include <disp/ssd1306.h>
#include <tty/fb_tty.h>
#include <tty/vt100.h>
#include <arch/linux/i2c_device.h>
#include <kernel/dev/framebuffer.h>

struct app {
	struct libk_device device; 
	i2c_dev_t bus; 
	fb_dev_t fb; 
	tty_dev_t tty; 
	serial_dev_t console; 
}; 

DEVICE_CLOCK(app_clock){
	struct libk_device *device = DEVICE_CLOCK_GET_DEVICE(); 
	struct app *self = container_of(device, struct app, device); 
	//static char buffer[128*8];
	
	ASYNC_BEGIN();
	while(1){
		//DEBUG("ssd: start\n"); 
		/*AWAIT(fbdev_open(self->fb)); 
		for(size_t c = 0; c < sizeof(buffer); c++) buffer[c] = 0xee; 
		fbdev_seek(self->fb, 0, 0); 
		fbdev_write(self->fb, (const uint8_t*)buffer, sizeof(buffer)); 
		DEVICE_DELAY(1000000); 
		for(size_t c = 0; c < sizeof(buffer); c++) buffer[c] = 0x0f; 
		fbdev_seek(self->fb, 0, 0); 
		fbdev_write(self->fb, (const uint8_t*)buffer, sizeof(buffer)); 
		fbdev_close(self->fb);
		DEVICE_DELAY(1000000);  
		//DEBUG("ssd: done!\n");
		*/
		//tty_move_cursor(self->tty, 0, 0); 
		//tty_put(self->tty, 'A', 1, 0);  
		//tty_move_cursor(self->tty, 4, 0); 
		//tty_put(self->tty, 'B', 1, 0); 
		serial_printf(self->console, "\x1b[2J\x1b[HHello World %d\n", 10); 
		ASYNC_YIELD(); 
	}
	ASYNC_END(0); 
}

static void __init app_init(void){
	static struct app app;
	static struct linux_i2c_device i2c_dev; 
	static struct ssd1306 ssd1306; 
	static struct fb_tty fbtty;
	static struct vt100 vt100;  
	DEBUG("imx23-app: init\n"); 
	linux_i2c_device_init(&i2c_dev, 0, SSD1306_I2C_ADDR);
	ssd1306_init(&ssd1306, linux_i2c_device_get_interface(&i2c_dev)); 
	fb_tty_init(&fbtty, ssd1306_get_gbuf(&ssd1306)); 
	app.tty = fb_tty_to_tty_device(&fbtty); 
	vt100_init(&vt100, app.tty); 
	app.console = vt100_to_serial_device(&vt100); 
	libk_register_device(&app.device, DEVICE_CLOCK_PTR(app_clock), "imx23-ssd1306"); 	
}
