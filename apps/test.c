#include <kernel.h>
#include <kernel/thread/process.h>
#include <pthread.h>


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; 

static char in_ch[255] = {0}; 

PROCESS(proc, "Main process"); 
PROCESS_THREAD(proc, ev, data){
	PROCESS_BEGIN(); 
	while(1){
		//int ch = serial_getc(std); 	
		//if(ch != SERIAL_NO_DATA) DEBUG("%x\n", ch); 
		//PROCESS_WAIT_UNTIL(ev == PROCESS_EVENT_POLL); 
		printf("got: %s\n", in_ch); 
		PROCESS_SLEEP(50000); 
		//PROCESS_PAUSE(); 
	}
	PROCESS_END(); 
}

static void input_irq(void){
	if(scanf("%s", in_ch) == 1){
		process_poll(&proc); 
	}
}

static void *irq_thread_proc(void *ptr){
	while(1){
		input_irq(); 
	}
	return 0; 
}

static int __init main_init(void){
	//struct libk_device device; 
	DEBUG("app init\n"); 
	//std = stdio_get_serial_interface(); 

	struct serial_driver *sdrv; 
	serial_for_each_driver(sdrv){	
		printf("serial: %s\n", sdrv->name); 
	}

	process_init(); 	
	process_start(&proc, 0); 

	pthread_t irq_thread; 
	pthread_create(&irq_thread, 0, &irq_thread_proc, 0); 
	process_post(&proc, PROCESS_EVENT_CONTINUE, 0); 

	while(1){
		timestamp_t t = timestamp_now(); 
		process_post(&proc, PROCESS_EVENT_CONTINUE, 0); 
		process_run(); 
		timestamp_t elapsed = timestamp_now()-t; 
		if(elapsed < 10) elapsed = 10 - elapsed; else elapsed = 0;  
		usleep(elapsed); 
		//printf("nev = %d\n", nev); 
		//ASYNC_RUN_PARALLEL(ASYNC_GLOBAL_QUEUE);  
	}

	return 0; 
}
