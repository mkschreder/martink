#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arch/soc.h>
#include <kernel.h>
#include "pipe.h"

static void *_linux_pipe_thread(void *arg){
	struct linux_pipe *self = (struct linux_pipe*)arg; 
	uint8_t buf[32]; 
	
	while(self->running){
		int ret = read(self->fd, buf, sizeof(buf)); 
		if(ret > 0) {
			pthread_mutex_lock(&self->buf_lock); 
			cbuf_putn(&self->buffer, buf, ret); 
			pthread_mutex_unlock(&self->buf_lock); 
			// signal event to main libk code 
			//libk_wake_up();
		}
	}

	return 0; 
}

/*
DEVICE_CLOCK(_pipe_clock){
	ASYNC_BEGIN(); 
	while(true){
		ASYNC_SUSPEND(); 
	}
	ASYNC_END(0); 
}*/

void linux_pipe_init(struct linux_pipe *self){
	self->fd = 0; 
	self->thread = 0; 
	self->running = 0; 
	cbuf_init(&self->buffer, self->_buffer_data, sizeof(self->_buffer_data)); 
	pthread_mutex_init(&self->buf_lock, NULL); 
}

int linux_pipe_open(struct linux_pipe *self, const char *path, linux_pipe_dir_t direction){
	//int ret; 
	//int mode = (direction == LINUX_PIPE_DIR_INPUT)?O_RDONLY:O_WRONLY; 

	//ret = mkfifo(path, 0666); 
	//if(ret < 0) return -1; 
	int fd = open(path, O_RDWR);
	if(fd < 0) {
		DEBUG("pipe: ENOENT\n"); 
		return -1; 
	}
	
	self->running = 1; 
	self->fd = fd; 

	if(pthread_create(&self->thread, NULL, &_linux_pipe_thread, self) != 0){
		close(fd);
		self->running = 0; 
		DEBUG("pipe: nothread\n"); 
		return -1; 
	}

	
	//libk_register_device(&self->device, DEVICE_CLOCK_PTR(_pipe_clock), "linux-pipe"); 

	return 0; 
}

static uint16_t _linux_pipe_get(serial_dev_t dev){
	struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 
	if(self->direction == LINUX_PIPE_DIR_INPUT){
		pthread_mutex_lock(&self->buf_lock); 
		uint16_t ret = cbuf_get(&self->buffer); 
		pthread_mutex_unlock(&self->buf_lock); 
		return ret; 
	}
	return 0; 
}

static uint16_t _linux_pipe_put(serial_dev_t dev, uint8_t ch){ 
	struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 
	if(self->direction == LINUX_PIPE_DIR_OUTPUT){
		pthread_mutex_lock(&self->buf_lock); 
		uint16_t ret = cbuf_put(&self->buffer, ch); 
		pthread_mutex_unlock(&self->buf_lock); 
		return ret;
	}
	return 0; 
}

static size_t	_linux_pipe_putn(serial_dev_t dev, const uint8_t *data, size_t max_sz){
	struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 
	size_t count = 0; 
	if(self->direction == LINUX_PIPE_DIR_OUTPUT){
		pthread_mutex_lock(&self->buf_lock); 
		count = cbuf_putn(&self->buffer, data, max_sz); 
		pthread_mutex_unlock(&self->buf_lock); 
	}
	return count; 
}

static size_t	_linux_pipe_getn(serial_dev_t dev, uint8_t *data, size_t max_sz){
	struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 
	size_t count = 0; 
	if(self->direction == LINUX_PIPE_DIR_INPUT){
		pthread_mutex_lock(&self->buf_lock); 
		count = cbuf_getn(&self->buffer, data, max_sz); 
		pthread_mutex_unlock(&self->buf_lock); 
	}
	return count; 
}

static int16_t	_linux_pipe_begin(serial_dev_t dev){ 
	//struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 
	return 0; 
}

static int16_t	_linux_pipe_end(serial_dev_t dev){
	//struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 

	return 0; 
}

static size_t 	_linux_pipe_waiting(serial_dev_t dev){
	struct linux_pipe *self = container_of(dev, struct linux_pipe, api); 
	if(self->direction == LINUX_PIPE_DIR_INPUT){
		pthread_mutex_lock(&self->buf_lock); 
		size_t num = cbuf_get_waiting(&self->buffer); 
		pthread_mutex_unlock(&self->buf_lock); 
		return num; 
	}
	return 0; 
}

serial_dev_t linux_pipe_to_serial_device(struct linux_pipe *self){
	static struct serial_if api = {
		.get = _linux_pipe_get, 
		.put = _linux_pipe_put, 
		.getn = _linux_pipe_getn, 
		.putn = _linux_pipe_putn,
		.begin = _linux_pipe_begin, 
		.end = _linux_pipe_end,
		.waiting = _linux_pipe_waiting
	}; 
	self->api = &api; 
	return &self->api; 
}

