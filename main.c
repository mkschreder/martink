#include "kernel.h"

extern void app_init(void); 
extern void app_loop(void); 

int main(void){
	brd_init(); 
	app_init(); 
	
	while(1){
		brd_process_events(); 
		app_loop(); 
		/*
		#ifdef RFNET
		rfnet_process_events(); 
		#endif
		
		#ifdef TCPIP
		ip_process_packets(); 
		#endif*/
	}
}

int __cxa_guard_acquire(__guard *g) {return !*(char *)(g);};
void __cxa_guard_release (__guard *g) {*(char *)g = 1;};
void __cxa_guard_abort (__guard *g) {};
void __cxa_pure_virtual(void) {};
