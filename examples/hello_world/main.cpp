/*
Example for basic setup and printing to the uart console on an AVR controller. 

Notice that you can just as well use LibK from C++ as from C. Therefore, 
this example is in C++. 
*/ 

#include <kernel.h>
#include <kernel/thread.h>
#include <kernel/cpp/thread.hpp>

class Application {
	friend PT_THREAD(_main_thread(struct libk_thread *kthread, struct pt *pt)); 
public:
	Application(); 
	void post_init(); 
private: 
	// serial device handle for the console. 
	serial_dev_t 	mConsole; 
	// the thread context that we will register with libk
	struct libk_thread thread; 
	timestamp_t timeout; 
	// counter 
	int 	mCounter; 
}; 

// we define buffers as part of our application always and pass them to devices
static uint8_t tx_buf[32]; 
static uint8_t rx_buf[32]; 

/*
 We define a free running protothread for the main loop. This thread method
 will be called automatically by libk as soon as it is defined like this 
 using LIBK_THREAD macro. We could also define it as PT_THREAD, but then 
 we would also need to call libk_create_thread in order to register the 
 thread with the scheduler. Note that all threads run to completion and so 
 a thread can never be interrupted - but we also need to structure threads
 using the PT_BEGIN and PT_END macros and only use PT_WAIT_XX macros when 
 we need to do a blocking wait (which will return the control to scheduler)
*/ 
LIBK_THREAD(ticker){
	static int ticker = 0; 
	PT_BEGIN(pt); 
	while(1){
		serial_printf(uart_get_serial_interface(0), "Tick: %d\n", ticker++); 
		// delays in libk are done using systick timer and a wait that will only
		// block this thread and allow other parts of the application to run. 
		static timestamp_t timeout = 0; 
		timeout = timestamp_from_now_us(500000); 
		PT_WAIT_UNTIL(pt, timestamp_expired(timeout)); 
	}
	PT_END(pt); 
}

PT_THREAD(_main_thread(struct libk_thread *kthread, struct pt *pt)){
	Application *app = cpp_container_of(kthread, &Application::thread); 
	//serial_printf(mConsole, "Hello World! %d seconds elapsed\n", mCounter++); 
	PT_BEGIN(pt); 
	while(1){
		serial_printf(app->mConsole, "Hello World!\n"); 
		// delays in libk are done using systick timer and a wait that will only
		// block this thread and allow other parts of the application to run. 
		app->timeout = timestamp_from_now_us(1000000); 
		PT_WAIT_UNTIL(pt, timestamp_expired(app->timeout)); 
	}
	PT_END(pt); 
	return 0; 
}

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
	
	uart_init(0, 38400, tx_buf, sizeof(tx_buf), rx_buf, sizeof(rx_buf)); 
	// we can no get the interface for the uart0 on the chip. 
	mConsole = uart_get_serial_interface(0); 
	
	// register the thread for this instance of app object
	libk_create_thread(&this->thread, _main_thread, "main"); 
}


int main(void){
	Application app; 
	
	libk_run(); 
}
