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

struct bdev {
	struct io_device io; 
	int tick;
};

void bdev_init(struct bdev *self){
	io_init(&self->io); 
}

PT_THREAD(bdev_open_async(struct pt *pt, io_dev_t dev)){
	struct bdev *self = container_of(dev, struct bdev, io); 

	PT_BEGIN(pt); 
	for(self->tick = 10; self->tick > 0; self->tick--){
		printf("opening\n"); 
		PT_YIELD(pt); 
	}
	printf("bdev open done\n"); 
	
	PT_END(pt); 
}

PT_THREAD(bdev_close_async(struct pt *pt, io_dev_t dev)){
	struct bdev *self = container_of(dev, struct bdev, io); 
	
	PT_BEGIN(pt); 
	for(self->tick = 10; self->tick > 0; self->tick--){
		PT_YIELD(pt); 
	}
	printf("bdev close done\n"); 
	PT_END(pt); 
}

PT_THREAD(bdev_write_async(struct pt *pt, io_dev_t dev, const uint8_t *data, ssize_t data_size)){
	struct bdev *self = container_of(dev, struct bdev, io); 
	int count = 0; 
	if(data_size >= BDEV_PACKET_SIZE) count = BDEV_PACKET_SIZE; 
	else count = data_size; 
	
	count = 2; 
	
	PT_BEGIN(pt); 
	while(1){
		
		for(int c = 0; c < count; c++){
			printf("%c", data[c]); 
		}
		printf("\n"); 
		
		if(_io_progress(&self->io, count) == 0) break; 
		
		PT_YIELD(pt); 
	}
	printf("bdev write done\n"); 
	PT_END(pt); 
}

PT_THREAD(bdev_read_async(struct pt *pt, io_dev_t dev, uint8_t *data, ssize_t data_size)){
	PT_BEGIN(pt); 
	PT_END(pt); 
}

PT_THREAD(bdev_seek_async(struct pt *pt, io_dev_t dev, ssize_t ofs, int whence)){
	PT_BEGIN(pt); 
	PT_END(pt); 
}

PT_THREAD(bdev_ioctl_async(struct pt *pt, io_dev_t dev, ioctl_req_t req, ...)){
	PT_BEGIN(pt); 
	PT_END(pt); 
}

io_dev_t bdev_get_io_interface(struct bdev *self){
	static struct io_device_ops _api; 
	if(!self->io.api){
		_api = (struct io_device_ops){
			.open = bdev_open_async, 
			.close = bdev_close_async, 
			.read = bdev_read_async, 
			.write = bdev_write_async, 
			.seek = bdev_seek_async, 
			.ioctl = bdev_ioctl_async
		};
	}
	self->io.api = &_api; 
	return &self->io; 
}

struct bdev bd; 
static int _run = 2; 

LIBK_THREAD(main_thread){
	io_dev_t io = bdev_get_io_interface(&bd); 
	
	static char buffer[32]; 
	sprintf(buffer, "Hello World!"); 
	
	PT_BEGIN(pt); 
	printf("first wait open\n"); 
	PT_ASYNC_BEGIN(pt, io, 1000);
	printf("first write\n"); 
	PT_ASYNC_SEEK(pt, io, 100000, 100, SEEK_SET); 
	if(io_error(io)){
		printf("seek1 failed: %s\n", strerror(-io_error(io))); 
		PT_ASYNC_END(pt, io, 1000);  
		PT_EXIT(pt); 
	}
	PT_ASYNC_WRITE(pt, io, 100000, buffer, strlen(buffer)); 
	if(io_error(io)){
		printf("Write1 failed: %s\n", strerror(-io_error(io))); 
		PT_ASYNC_END(pt, io, 1000);  
		PT_EXIT(pt); 
	} 
	printf("writing some more\n"); 
	PT_ASYNC_WRITE(pt, io, 100000, buffer, strlen(buffer)); 
	PT_ASYNC_END(pt, io, 1000);  
	_run--; 
	PT_END(pt); 
}

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

int main(void){
	printf("STARTING..\n"); 
	printf("Size: %lu\n", sizeof(struct io_device) + sizeof(struct io_device_ops)); 
	
	bdev_init(&bd); 
	
	while(_run){
		libk_schedule(); 
	}
	
	printf("Done\n"); 
}
