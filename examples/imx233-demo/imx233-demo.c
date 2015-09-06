#include <kernel.h>
#include <kernel/thread.h>
#include <boards/imx233/imx23.h>
#include <arch/native/fd_serial.h>

struct application {
	pio_dev_t gpio; 
	serial_dev_t console; 
	struct async_process process; 
	timestamp_t timeout; 
}; 

struct application app; 

ASYNC_PROCESS(_main_process){
	struct application *appl = container_of(__self, struct application, process); 
	ASYNC_BEGIN(); 
	while(1){
		pio_set_pin(appl->gpio, 50); 
		AWAIT_DELAY(appl->timeout, 1000000L); 
		pio_clear_pin(appl->gpio, 50); 
		AWAIT_DELAY(appl->timeout, 1000000L); 
	}
	ASYNC_END(0); 
}

int main(void){
	struct imx23_gpio gpio; 
	imx23_gpio_init(&gpio); 
	
	app.gpio = imx23_gpio_get_parallel_interface(&gpio); 
	app.console = stdio_get_serial_interface(); 
	
	pio_configure_pin(app.gpio, 50, GP_OUTPUT); 
	
	libk_init_process(&app.process, _main_process); 
	libk_register_process(&app.process); 
	
	libk_loop(); 
}
