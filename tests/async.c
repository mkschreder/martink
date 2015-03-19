#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include <kernel/thread.h>
#include "../kernel/thread.c"
#include "../arch/native/time.c"
#include "../arch/time.c"
#include "../kernel/io_device.c"

#include <string.h>

#define BDEV_PACKET_SIZE 8

enum {
	BDEV_STATE_CLOSED, 
	BDEV_STATE_OPEN
}; 

typedef struct bdev {
	struct io_device io; 
	ssize_t size; 
	int tick;
} bdev_t;

void bdev_init(struct bdev *self){
	io_init(&self->io); 
}

static ASYNC(io_device_t, open){
	struct bdev *dev = container_of(self, struct bdev, io); 

	ASYNC_BEGIN(); 
	// here lock mutex for open/close for exclusive access
	// ...
	
	for(dev->tick = 10; dev->tick > 0; dev->tick--){
		printf("opening\n"); 
		ASYNC_YIELD(); 
	}
	printf("bdev open done\n"); 
	
	ASYNC_END(); 
}

static ASYNC(io_device_t, close){
	struct bdev *dev = container_of(self, struct bdev, io); 
	
	ASYNC_BEGIN(); 
	for(dev->tick = 10; dev->tick > 0; dev->tick--){
		ASYNC_YIELD(); 
	}
	printf("bdev close done\n"); 
	ASYNC_END(); 
}

static ASYNC(io_device_t, write, const uint8_t *data, ssize_t data_size)){
	struct bdev *dev = container_of(self, struct bdev, io); 
	if(data_size >= BDEV_PACKET_SIZE) data_size = BDEV_PACKET_SIZE; 
	
	ASYNC_BEGIN(); 
	
	dev->size = data_size; 
	
	while(dev->size){
		for(int c = 0; c < data_size && dev->size; c++, dev->size--){
			printf("%c", data[c]); 
		}
		printf("\n"); 
		
		ASYNC_YIELD(); 
	}
	printf("bdev write done\n"); 
	
	ASYNC_END(); 
}

static ASYNC(io_device_t, read, uint8_t *data, ssize_t data_size)){
	ASYNC_BEGIN(); 
	ASYNC_END(); 
}

static ASYNC(io_device_t, seek, ssize_t ofs, int whence)){
	ASYNC_BEGIN(); 
	ASYNC_END(); 
}

static ASYNC(io_device_t, ioctl, ioctl_req_t req, ...)){
	ASYNC_BEGIN(); 
	ASYNC_END(); 
}

io_dev_t bdev_get_io_interface(struct bdev *self){
	static struct io_device_ops _api; 
	if(!self->io.api){
		_api = (struct io_device_ops){
			.open = __io_device_t_open__, 
			.close = __io_device_t_close__, 
			.read = __io_device_t_read__, 
			.write = __io_device_t_write__, 
			.seek = __io_device_t_seek__, 
			.ioctl = __io_device_t_ioctl__
		};
	}
	self->io.api = &_api; 
	return &self->io; 
}

struct bdev bd; 
static int _run = 2; 

typedef struct application {
	struct async_task main; 
} app_t; 

void app_init(struct application *self){
	ASYNC_INIT(&self->main); 
}

ASYNC(app_t, main){
	io_dev_t io = bdev_get_io_interface(&bd); 
	
	static char buffer[32]; 
	sprintf(buffer, "Hello World!"); 
	
	ASYNC_BEGIN(); 
	
	printf("first wait open\n"); 
	AWAIT_TASK(io_device_t, open, io);
	printf("first write\n"); 
	AWAIT_TASK(io_device_t, seek, io, 100, SEEK_SET); 
	/*if(io_error(io)){
		printf("seek1 failed: %s\n", strerror(-io_error(io))); 
		PT_ASYNC_END(pt, io, 1000);  
		PT_EXIT(pt); 
	}*/
	AWAIT_TASK(io_device_t, write, io, buffer, strlen(buffer)); 
	/*if(io_error(io)){
		printf("Write1 failed: %s\n", strerror(-io_error(io))); 
		PT_ASYNC_END(pt, io, 1000);  
		PT_EXIT(pt); 
	} */
	printf("writing some more\n"); 
	AWAIT_TASK(io_device_t, write, io, buffer, strlen(buffer)); 
	AWAIT_TASK(io_device_t, close, io);  
	_run--; 
	ASYNC_END(); 
}
/*
LIBK_THREAD(second_thread){
	io_dev_t io = bdev_get_io_interface(&bd); 
	
	static char buffer[32]; 
	sprintf(buffer, "Second"); 
	
	PT_BEGIN(pt); 
	printf("second wait open\n"); 
	PT_WAIT_UNTIL(pt, io_open(io, 1000));
	printf("second write\n"); 
	PT_WAIT_UNTIL(pt, io_write(io, 1000, buffer, strlen(buffer))); 
	if(io_error(io)){
		printf("Write2 failed!\n"); 
	} 
	PT_WAIT_UNTIL(pt, io_close(io, 1000));  
	_run--; 
	PT_END(pt); 
}
*/
int main(void){
	printf("STARTING..\n"); 
	printf("Size: %lu\n", sizeof(struct io_device) + sizeof(struct io_device_ops)); 
	struct application app; 
	app_init(&app); 
	bdev_init(&bd); 
	
	while(_run){
		ASYNC_INVOKE_ONCE(app_t, main, 0, &app); 
	}
	
	printf("Done\n"); 
}
