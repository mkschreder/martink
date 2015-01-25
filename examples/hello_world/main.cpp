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
	// timeout value for the delay
	timestamp_t 	mNextPrintTime; 
	// counter 
	int 	mCounter; 
}; 

Application::Application():mConsole(0), mNextPrintTime(0), mCounter(0){
	/*
	do initialization of on chip devices. These depend on the chip being 
	compiled for. All these devices are implemented in arch/ folder. 
	
	If you are compiling for a specific board such as for example 
	multiwii flight controller, then you would call board specific init 
	function that in turn should initialize the soc as well. 
	
	This method is not called automatically for the sake of being more 
	explicit about how libk is initialized. 
	*/
	soc_init(); 
	
	// we can no get the interface for the uart0 on the chip. 
	mConsole = uart_get_serial_interface(0); 
	
	// we use the timestamp system to set next print time to expire right away. 
	// timestamps use hardware timer for keeping track of "ticks". Usually 
	// one or few ticks per microsecond. Therefore all timeouts have 
	// precision of up to 1 microsecond. 
	mNextPrintTime = timestamp_now(); 
}

int Application::loop(){
	// we have here an async loop that prints a message every second
	if(timestamp_expired(mNextPrintTime)){
		// it would probably be better to 
		serial_printf(mConsole, "Hello World! %d seconds elapsed\n", mCounter++); 
		// set one second timeout until next time
		mNextPrintTime = timestamp_from_now_us(1000000UL); 
	}
}

int main(void){
	Application app; 
	
	while(1){
		app.loop(); 
	}
}
