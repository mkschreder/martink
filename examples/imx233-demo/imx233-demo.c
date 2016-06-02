#include <kernel.h>
#include <kernel/thread.h>
#include <boards/imx233/imx23.h>
#include <arch/linux/fd_serial.h>

struct application {
	pio_dev_t gpio; 
	serial_dev_t console; 
	struct async_process process; 
	struct imx23_gpio imx_gpio; 
	timestamp_t timeout; 
}; 

static struct application app; 

ASYNC_PROCESS(_main_process){
	struct application *self = container_of(__self, struct application, process); 
	ASYNC_BEGIN(); 
	while(1){
		serial_printf(self->console, "HIGH\n"); 
		pio_set_pin(self->gpio, 50); 
		AWAIT_DELAY(self->timeout, 1000000L); 
		serial_printf(self->console, "LOW\n"); 
		pio_clear_pin(self->gpio, 50); 
		AWAIT_DELAY(self->timeout, 1000000L); 
	}
	ASYNC_END(0); 
}

static void __init imx23_demo_init(void){
	imx23_gpio_init(&app.imx_gpio); 
	
	app.gpio = imx23_gpio_get_parallel_interface(&app.imx_gpio); 
	app.console = stdio_get_serial_interface(); 
	
	serial_printf(app.console, "Will set bank %d pin %d\n", 50 >> 5, 50 & 0x1f); 
	
	pio_configure_pin(app.gpio, 50, GP_OUTPUT); 
	
	libk_init_process(&app.process, _main_process); 
	libk_register_process(&app.process); 
}

