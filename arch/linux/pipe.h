#pragma once

#include <kernel/dev/serial.h>
#include <kernel/cbuf.h>
#include <kernel.h>

#include <pthread.h>

typedef enum {
	LINUX_PIPE_DIR_INPUT, 
	LINUX_PIPE_DIR_OUTPUT
}linux_pipe_dir_t; 

struct linux_pipe {
	int fd; 
	linux_pipe_dir_t direction; 
	struct libk_device device; 
	pthread_t thread;
	pthread_mutex_t buf_lock; 
	uint8_t _buffer_data[1024]; 
	struct cbuf buffer; 
	uint8_t running; 
	struct serial_device_ops *api; 
}; 

void linux_pipe_init(struct linux_pipe *self); 
int linux_pipe_open(struct linux_pipe *self, const char *path, linux_pipe_dir_t direction); 
//void linux_pipe_register_event(struct linux_pipe *self, linux_pipe_event_t ev, void (*event)(void)); 
serial_dev_t linux_pipe_to_serial_device(struct linux_pipe *self); 
