#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include "../kernel/dev/block.h"
#include <kernel/thread.h>
#include "../block/rd.c"
#include "../kernel/thread.c"
#include "../arch/native/time.c"
#include "../arch/time.c"
#include "../kernel/transfer.c"
#include "../block/i2cblk.c"
#include "../sensors/bmp085.c"
#include "../block/block_device.c"

#define BLOCK_SIZE 32
#define BLOCK_COUNT 2

struct app {
	struct bmp085 bmp, bmp2; 
	block_dev_t rd; 
	struct block_transfer tr; 
	uint8_t buffer[BLOCK_SIZE * BLOCK_COUNT]; 
}; 

static struct app _app; 

LIBK_THREAD(main){
	PT_BEGIN(pt); 
	while(1){
		static timestamp_t time = 0; 
		static int ticks = 0; 
		
		time = timestamp_from_now_us(1000000); 
		printf("Tick: %d\n", ticks++); 
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
		
	}
	PT_END(pt); 
}

int main(){
	struct ramdisk rd; 
	struct i2c_block_device i2cblk; 
	
	uint8_t buffer[256]; 
	rd_init(&rd, buffer, 256, 1); 
	
	// create a dummy i2cblk device based on the ramdisk underneath so we can debug it
	i2cblk_init(&i2cblk, rd_get_interface(&rd), BMP085_ADDR); 
	
	bmp085_init(&_app.bmp, i2cblk_get_interface(&i2cblk)); 
	bmp085_init(&_app.bmp2, i2cblk_get_interface(&i2cblk)); 
	
	libk_run(); 
}
