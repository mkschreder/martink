#include <kernel.h>
#include <disp/ssd1306.h>
#include <disp/ili9340.h>
#include <tty/fb_tty.h>
#include <tty/vt100.h>
#include <arch/linux/i2c_device.h>
#include <arch/linux/pipe.h>
#include <arch/linux/spidev.h>
#include <kernel/dev/framebuffer.h>

struct app {
	struct libk_device device; 
	i2c_dev_t bus; 
	fb_dev_t fb; 
	tty_dev_t tty; 
	serial_dev_t console; 
	serial_dev_t input; 
	spi_dev_t spi; 
}; 

DEVICE_CLOCK(app_clock){
	struct libk_device *device = DEVICE_CLOCK_GET_DEVICE(); 
	struct app *self = container_of(device, struct app, device); 
	static uint8_t buffer[128];
	
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
		//AWAIT(serial_get_waiting(app.input) > 0); 
		if(serial_waiting(self->input)){
			size_t ret = serial_getn(self->input, buffer, sizeof(buffer)); 
			if(ret > 0){
				DEBUG("in: %s\n", buffer); 
				serial_putn(self->console, buffer, ret); 
			}
		}
		uint8_t buf[2] = {0xaa, 0}; 
		spi_transfer(self->spi, buf, buf+1, 1); 
		/*
		tty_move_cursor(self->tty, 0, 0); 
		tty_put(self->tty, 'A', 1, 0);  
		//tty_move_cursor(self->tty, 4, 0); 
		//tty_put(self->tty, 'B', 1, 0); 
		//serial_printf(self->console, "\x1b[2J\x1b[HWorld is ours! %d\n", 10); 
		DEVICE_DELAY(500000);  
		tty_move_cursor(self->tty, 0, 0); 
		tty_put(self->tty, 'B', 1, 0);  
		DEVICE_DELAY(500000); 
		serial_printf(self->console, "\x1b[2J\x1b[HHello World %d\n", 10); 
		DEVICE_DELAY(500000);  
		*/
		DEVICE_DELAY(100000); 
	}
	ASYNC_END(0); 
}
/*
DEVICE_EVENT(pipe_input){
	ASYNC_BEGIN();
	while(serial_get_waiting(app.input) > 0) serial_put(app.console, serial_get(app.input)); 
	ASYNC_END(0); 
}
*/
static void __init _z_app_init(void){
	static struct app app;
	static struct linux_i2c_device i2c_dev; 
	static struct ssd1306 ssd1306; 
	static struct fb_tty fbtty;
	static struct vt100 vt100;  
	static struct linux_pipe lpipe; 
	async_queue_init(&ASYNC_GLOBAL_QUEUE); 
	static struct linux_spidev spi; 
	DEBUG("imx23-app: init\n"); 
	linux_i2c_device_init(&i2c_dev, 0, SSD1306_I2C_ADDR);
	ssd1306_init(&ssd1306, linux_i2c_device_get_interface(&i2c_dev)); 
	fb_tty_init(&fbtty, ssd1306_get_gbuf(&ssd1306)); 
	app.tty = fb_tty_to_tty_device(&fbtty); 
	vt100_init(&vt100, app.tty); 
	app.console = vt100_to_serial_device(&vt100); 
	linux_pipe_init(&lpipe);
	linux_pipe_open(&lpipe, "/var/ssd1306", LINUX_PIPE_DIR_INPUT);  
	linux_spidev_init(&spi); 
	app.spi = linux_spidev_to_interface(&spi); 
//void ili9340_init(struct ili9340 *self, spi_dev_t spi, pio_dev_t gpio, gpio_pin_t cs_pin, gpio_pin_t dc_pin, gpio_pin_t rst_pin) {
	 
	//linux_pipe_register_event(LINUX_PIPE_EV_INPUT_READY, pipe_event); 
	app.input = linux_pipe_to_serial_device(&lpipe); 
	libk_register_device(&app.device, DEVICE_CLOCK_PTR(app_clock), "imx23-ssd1306"); 	
}
