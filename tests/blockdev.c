#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include <kernel/thread.h>
#include "../kernel/io_device.c"
#include "../block/rd.c"
#include "../kernel/thread.c"
#include "../arch/native/time.c"
#include "../arch/time.c"
#include "../block/i2cblk.c"

#define BLOCK_SIZE 32
#define BLOCK_COUNT 2

struct app {
	io_dev_t rd; 
	uint8_t buffer[BLOCK_SIZE * BLOCK_COUNT]; 
}; 

static struct app _app; 

#define TIMEOUT 500000

LIBK_THREAD(main){
	PT_BEGIN(pt); 
	while(1){
		static timestamp_t time = 0; 
		static int ticks = 0; 
		
		time = timestamp_from_now_us(1000000); 
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
		printf("Tick: %d\n", ticks++); 
		
		PT_ASYNC_BEGIN(pt, _app.rd, TIMEOUT); 
		
		printf("Writing data..\n"); 
		for(unsigned int c = 0; c < sizeof(_app.buffer); c++){
			_app.buffer[c] = c; 
		}
		
		PT_ASYNC_WRITE(pt, _app.rd, TIMEOUT, _app.buffer, sizeof(_app.buffer)); 
		
		printf("Reading data..\n"); 
		memset(_app.buffer, 0, sizeof(_app.buffer)); 
		
		PT_ASYNC_SEEK(pt, _app.rd, TIMEOUT, 0, SEEK_SET); 
		PT_ASYNC_READ(pt, _app.rd, TIMEOUT, _app.buffer, sizeof(_app.buffer));
		
		PT_ASYNC_END(pt, _app.rd, TIMEOUT); 
		 
		for(unsigned int c = 0; c < sizeof(_app.buffer); c++){
			if(_app.buffer[c] != c){
				printf("Data integrity check failed at %d: expected %d, got %d\n", c, c, _app.buffer[c]); 
				PT_WAIT_WHILE(pt, 1); 
			}
		}
		
	}
	PT_END(pt); 
}

int main(){
	struct ramdisk rd; 
	uint8_t buffer[BLOCK_SIZE * BLOCK_COUNT]; 
	rd_init(&rd, buffer, BLOCK_SIZE * BLOCK_COUNT, BLOCK_COUNT); 
	_app.rd = rd_get_interface(&rd); 
	
	libk_run(); 
}
