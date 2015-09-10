#include <kernel.h>
#include <disp/ssd1306.h>
#include <arch/linux/i2c_device.h>

struct app {
	struct libk_device device; 

}; 

DEVICE_CLOCK(app_clock){
	struct libk_device *device = DEVICE_CLOCK_GET_DEVICE(); 
	struct app *self = container_of(device, struct app, device); 
	ASYNC_BEGIN(); 
	(void)self; 
	ASYNC_END(0); 
}

static void __init app_init(void){
	static struct app app; 
	libk_register_device(&app.device, DEVICE_CLOCK_PTR(app_clock), "imx23-ssd1306"); 
	
}
