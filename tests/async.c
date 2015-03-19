#define CONFIG_NATIVE 1

#include "../arch/soc.h"
#include <kernel/thread.h>
#include "../kernel/thread.c"
#include "../arch/native/time.c"
#include "../arch/time.c"
#include "../kernel/io_device.h"

#include <string.h>

#define BDEV_PACKET_SIZE 2

enum {
	BDEV_STATE_CLOSED, 
	BDEV_STATE_OPEN
}; 

typedef struct bdev {
	struct io_device io; 
	ssize_t size; 
	int tick;
	uint8_t _lock; 
} bdev_t;

void bdev_init(struct bdev *self){
	io_init(&self->io); 
	ASYNC_MUTEX_INIT(self->_lock, 1); 
}

static ASYNC(io_device_t, open){
	struct bdev *dev = container_of(self, struct bdev, io); 

	ASYNC_BEGIN(); 
	
	ASYNC_MUTEX_LOCK(dev->_lock); 
	
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
		printf("closing\n"); 
		ASYNC_YIELD(); 
	}
	ASYNC_MUTEX_UNLOCK(dev->_lock); 
	
	printf("bdev close done\n"); 
	ASYNC_END(); 
}

static ASYNC(io_device_t, write, const uint8_t *data, ssize_t data_size){
	struct bdev *dev = container_of(self, struct bdev, io); 
	//if(data_size >= BDEV_PACKET_SIZE) size = BDEV_PACKET_SIZE; 
	//else size = data_size; 
	
	ASYNC_BEGIN(); 
	
	dev->size = data_size; 
	
	while(dev->size){
		int offset = data_size - dev->size; 
		int end = offset + BDEV_PACKET_SIZE; 
		if(end > data_size) end = data_size; 
		for(; 
			offset < end && dev->size; 
			offset++, dev->size--){
			printf("%c", data[offset]); 
		}
		printf("\n"); 
		
		ASYNC_YIELD(); 
	}
	printf("bdev write done\n"); 
	
	ASYNC_END(); 
}

static ASYNC(io_device_t, read, uint8_t *data, ssize_t data_size){
	ASYNC_BEGIN(); 
	ASYNC_END(); 
}

static ASYNC(io_device_t, seek, ssize_t ofs, int whence){
	ASYNC_BEGIN(); 
	ASYNC_END(); 
}

static ASYNC(io_device_t, ioctl, ioctl_req_t req, ...){
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
	struct async_task main, second; 
} app_t; 

void app_init(struct application *self){
	ASYNC_INIT(&self->main); 
	ASYNC_INIT(&self->second); 
}

ASYNC(app_t, main){
	io_dev_t io = bdev_get_io_interface(&bd); 
	static _done = 0; 
	if(_done) return ASYNC_ENDED; 
	static char buffer[32]; 
	sprintf(buffer, "Hello World!"); 
	
	ASYNC_BEGIN(); 
	
	printf("first wait open\n"); 
	IO_OPEN(io); 
	printf("first write\n"); 
	IO_SEEK(io, 100, SEEK_SET); 
	IO_WRITE(io, buffer, strlen(buffer)); 
	printf("writing some more\n"); 
	IO_WRITE(io, buffer, strlen(buffer)); 
	IO_CLOSE(io);  
	_run--; 
	_done = 1; 
	
	ASYNC_END(); 
}

ASYNC(app_t, second){
	io_dev_t io = bdev_get_io_interface(&bd); 
	static _done = 0; 
	if(_done) return ASYNC_ENDED; 
	static char buffer[32]; 
	sprintf(buffer, "Second"); 
	
	ASYNC_BEGIN(); 
	
	printf("second wait open\n"); 
	IO_OPEN(io);
	printf("second write\n"); 
	IO_WRITE(io, buffer, strlen(buffer)); 
	IO_CLOSE(io);  
	_run--; 
	_done = 1; 
	
	ASYNC_END(); 
}

int main(void){
	printf("STARTING..\n"); 
	printf("Size: %lu\n", sizeof(struct io_device) + sizeof(struct io_device_ops)); 
	struct application app; 
	app_init(&app); 
	bdev_init(&bd); 
	
	while(_run){
		ASYNC_INVOKE_ONCE(app_t, main, 0, &app); 
		ASYNC_INVOKE_ONCE(app_t, second, 0, &app); 
	}
	
	printf("Done\n"); 
}
