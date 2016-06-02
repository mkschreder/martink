/*
Example for basic setup and printing to the uart console on an AVR controller. 

Notice that you can just as well use LibK from C++ as from C. Therefore, 
this example is in C++. 
*/ 

#include <kernel.h>
#include <kernel/thread.h>
#include <arch/native/fd_serial.h>

//#include <kernel/cpp/thread.hpp>

struct application {
	// serial device handle for the console. 
	serial_dev_t 	console; 
	// the thread context that we will register with libk
	struct async_process process, ticker; 
	timestamp_t timeout; 
	// counter 
	int 	mCounter; 
}; 

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
ASYNC_PROCESS(ticker){
	struct application *app = container_of(__self, struct application, ticker); 
	static int ticker = 0; 
	ASYNC_BEGIN(); 
	while(1){
		serial_printf(app->console, "Tick: %d\n", ticker++); 
		// delays in libk are done using systick timer and a wait that will only
		// block this thread and allow other parts of the application to run. 
		static timestamp_t timeout = 0; 
		timeout = timestamp_from_now_us(500000); 
		AWAIT(timestamp_expired(timeout)); 
	}
	ASYNC_END(0); 
}

ASYNC_PROCESS(_main_thread){
	struct application *app = container_of(__self, struct application, process); 
	//serial_printf(mConsole, "Hello World! %d seconds elapsed\n", mCounter++); 
	ASYNC_BEGIN(); 
	while(1){
		serial_printf(app->console, "Hello World!\n"); 
		// delays in libk are done using systick timer and a wait that will only
		// block this thread and allow other parts of the application to run. 
		app->timeout = timestamp_from_now_us(1000000); 
		AWAIT(timestamp_expired(app->timeout)); 
	}
	ASYNC_END(0); 
}

int main(void){
	struct application app; 
	
	// we can no get the interface for the uart0 on the chip. 
	app.console = stdio_get_serial_interface(); 
	
	ASYNC_PROCESS_INIT(&app.process, _main_thread); 
	ASYNC_PROCESS_INIT(&app.ticker, ticker); 
	ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, &app.process); 
	ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, &app.ticker); 
	
	while(ASYNC_RUN_PARALLEL(&ASYNC_GLOBAL_QUEUE)); 
}
