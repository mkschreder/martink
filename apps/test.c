#include <kernel.h>
#include <kernel/errno.h>
#include <kernel/thread/process.h>
#include <kernel/thread/pt.h>
#include <pthread.h>
#include <stdbool.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 

static char in_ch[255] = {0}; 

#define UART_START_TX _BV(1)
#define UART_TX_READY _BV(2)
#define UART_RX_READY _BV(3)
#define UART_BUSY _BV(4)

struct soft_uart {
	uint8_t data; 
	uint8_t rx_data; 
	uint8_t flags; 
	pthread_mutex_t lock; 

	const char *buffer; 
	uint8_t start; 
	uint8_t len; 
}; 

static struct soft_uart _uart = {
	.data = 0, 
	.flags = 0, 
	.lock = PTHREAD_MUTEX_INITIALIZER
}; 

PROCESS(soft_uart_proc, "SoftUART process"); 
PROCESS_THREAD(soft_uart_proc, ev, data){
	PROCESS_BEGIN(); 
	while(1){
		PROCESS_WAIT_EVENT(); 
		pthread_mutex_lock(&_uart.lock); 
		printf("sending data to uart interrupt..\n"); 
		for(_uart.start = 0; _uart.start < _uart.len; _uart.start++){
			_uart.data = _uart.buffer[_uart.start]; 
			_uart.flags |= UART_START_TX;  
			pthread_mutex_unlock(&_uart.lock); 
			PROCESS_WAIT_UNTIL(_uart.flags & UART_TX_READY); 
			pthread_mutex_lock(&_uart.lock); 
			_uart.flags &= ~UART_TX_READY; 
		}
		_uart.flags &= ~UART_BUSY; 
		printf("all chars sent!\n"); 
		pthread_mutex_unlock(&_uart.lock); 
	}
	PROCESS_END(); 
}

// an example imaginary uart that runs independently and consumes data that a process writes to it. 
static void *soft_uart_hw_thread(void *ptr){
	struct pt pt; 
	while(1){
		if(_uart.flags & UART_START_TX) {
			printf("\e[2;%dH%c", 1+_uart.start, _uart.data); 
			fflush(stdout); 
			pthread_mutex_lock(&_uart.lock); 
			_uart.flags &= ~UART_START_TX; 
			_uart.flags |= UART_TX_READY; 
			pthread_mutex_unlock(&_uart.lock); 
			process_post(&soft_uart_proc, PROCESS_EVENT_CONTINUE, 0); 
		}
		usleep(100000); // it only supports baud rate of 10 chars per second
	}
	return 0; 
}

// an example imaginary uart that runs independently and consumes data that a process writes to it. 
static void *soft_uart_rx_thread(void *ptr){
	struct pt pt; 
	while(1){
		_uart.rx_data = getchar(); 
		pthread_mutex_lock(&_uart.lock); 
		_uart.flags |= UART_RX_READY; 
		pthread_mutex_unlock(&_uart.lock); 
		// in this example we are using low freq polling so if we do not sleep here, the program will miss incoming data
		usleep(100000); 
	}
	return 0; 
}

static int soft_uart_print(const char *str){
	// this is a way to implement fully blocking call 
	// however, for best result one should never use direct calls like this 
	// this will never be able to go into powersave mode while waiting for hardware to finish transmission
	#if 0
	while(_uart.flags & UART_BUSY) {
		process_post_sync(&soft_uart_proc, PROCESS_EVENT_CONTINUE, 0); 
	}
	#endif
	// this is another alternative
	if(_uart.flags & UART_BUSY) {
		return -EAGAIN; 
	}
	pthread_mutex_lock(&_uart.lock); 
	printf("buffering more data..\n"); 
	_uart.buffer = str; 
	_uart.start = 0; 
	_uart.len = strlen(str); 
	_uart.flags |= UART_BUSY; 
	pthread_mutex_unlock(&_uart.lock); 
	process_post(&soft_uart_proc, PROCESS_EVENT_CONTINUE, 0); 
	return 0; 
}

static bool soft_uart_ready(void){
	return !(_uart.flags & UART_BUSY); 
}

PROCESS(proc, "Main process"); 
PROCESS_THREAD(proc, ev, data){
	PROCESS_BEGIN(); 
	PROCESS_WAIT_EVENT(); 
	while(1){
		soft_uart_print("Hello World!\n"); 
		PROCESS_WAIT_UNTIL(soft_uart_ready()); 
		soft_uart_print("This is another string!\n"); 
		PROCESS_WAIT_UNTIL(soft_uart_ready()); 
		PROCESS_SLEEP(100000); 
	}
	PROCESS_END(); 
}

PROCESS(input_proc, "Input process"); 
PROCESS_THREAD(input_proc, ev, data){
	static int start_pos = 0; 
	PROCESS_BEGIN(); 
	PROCESS_WAIT_EVENT(); 
	while(1){
		PROCESS_WAIT_UNTIL(_uart.flags & UART_RX_READY); 
		char ch = _uart.rx_data; 
		printf("\e[2;%dH%c\n", 32 + start_pos, ch); 
		start_pos++; 
		if(ch == '\n') start_pos = 0; 
		pthread_mutex_lock(&_uart.lock); 
		_uart.flags &= ~UART_RX_READY; 
		pthread_mutex_unlock(&_uart.lock); 
	}
	PROCESS_END(); 
}

static int __init main_init(void){
	//struct libk_device device; 
	DEBUG("app init\n"); 
	printf("\e[2J\n"); 
	//std = stdio_get_serial_interface(); 

	struct serial_driver *sdrv; 
	serial_for_each_driver(sdrv){	
		printf("serial: %s\n", sdrv->name); 
	}

	//struct fb_driver *disp = framebuffer_get_default(); 

	pthread_t irq_thread; 
	pthread_t rx_thread; 
	pthread_create(&irq_thread, 0, &soft_uart_hw_thread, 0); 
	pthread_create(&rx_thread, 0, &soft_uart_rx_thread, 0); 

	process_init(); 	
	process_start(&proc, 0); 
	process_start(&soft_uart_proc, 0); 
	process_start(&input_proc, 0); 

	process_post(&proc, PROCESS_EVENT_CONTINUE, 0); 

	#define FRAME_TIME 10000
	int32_t usage = 0;  
	int32_t frames = 0; 
	while(1){
		timestamp_t t = timestamp_now(); 
		//process_post(&proc, PROCESS_EVENT_CONTINUE, 0); 
		process_poll(&proc); 
		process_poll(&input_proc); 
		process_run(); 
		timestamp_t elapsed = timestamp_now()-t; 
		usage+=elapsed; frames++;  
		if(elapsed < FRAME_TIME) elapsed = FRAME_TIME - elapsed; else elapsed = 0;  
		if(frames == (1000000/FRAME_TIME)){
			printf("\e[20;1HCPU usage %f\n", ((float)usage / frames) / FRAME_TIME); 
			frames = 0; 
			usage = 0; 
		}
		usleep(elapsed); 
		//printf("nev = %d\n", elapsed); 
		//ASYNC_RUN_PARALLEL(ASYNC_GLOBAL_QUEUE);  
	}

	return 0; 
}
