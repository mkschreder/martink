/*
Example for basic setup and printing to the uart console on an AVR controller. 

Notice that you can just as well use LibK from C++ as from C. Therefore, 
this example is in C++. 
*/ 

#include <kernel.h>

class Application {
public:
	Application(); 
	void post_init(); 
	int loop(); 
private: 
	// serial device handle for the console. 
	serial_dev_t 	mConsole; 
	// counter 
	int 	mCounter; 
}; 

Application::Application():mConsole(0), mCounter(0){
	/*
	do initialization of on chip devices. These depend on the chip being 
	compiled for. All these devices are implemented in arch/ folder. 
	
	If you are compiling for a specific board such as for example 
	multiwii flight controller, then you would call board specific init 
	function that in turn should initialize the soc as well. 
	
	This method is not called automatically for the sake of being more 
	explicit about how libk is initialized. 
	*/
	//soc_init(); 
	
	uart0_init(38400); 
	// we can no get the interface for the uart0 on the chip. 
	mConsole = uart_get_serial_interface(0); 
}

int Application::loop(){
	//serial_printf(mConsole, "Hello World! %d seconds elapsed\n", mCounter++); 
	serial_printf(mConsole, "Hello World!\n"); 
	for(volatile int c = 0; c < 10000; c++) mCounter++; 
	return 0; 
}

int main(void){
	Application app; 
	
	while(1){
		app.loop(); 
	}
}
