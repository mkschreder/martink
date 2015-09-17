#include <kernel.h>

static serial_dev_t std; 

DEVICE_CLOCK(_main_clock){
	ASYNC_BEGIN(); 
	while(1){
		int ch = serial_getc(std); 	
		if(ch != SERIAL_NO_DATA) DEBUG("%x\n", ch); 
		DEVICE_DELAY(10); 
	}
	ASYNC_END(0); 
}

static int __init main_init(void){
	struct libk_device device; 
	DEBUG("app init\n"); 
	std = stdio_get_serial_interface(); 
	libk_register_device(&device, DEVICE_CLOCK_PTR(_main_clock), "test-app"); 
	libk_loop(); 
	return 0; 
}
