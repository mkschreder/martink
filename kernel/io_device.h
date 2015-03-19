
/*
 Base class for a block device. Basic open / close state management
 */
 
#pragma once

#include <kernel/dev/block.h>
#include <kernel/thread.h>
#include <stdarg.h>

typedef struct io_device *io_dev_t; 

struct io_device; 

struct io_device_ops {
	async_return_t (*open)(struct async_task *parent, struct io_device *self); 
	async_return_t (*close)(struct async_task *parent, struct io_device *self); 
	async_return_t (*write)(struct async_task *parent, struct io_device *self, const uint8_t *data, ssize_t data_size); 
	async_return_t (*read)(struct async_task *parent, struct io_device *self, uint8_t *data, ssize_t data_size); 
	async_return_t (*seek)(struct async_task *parent, struct io_device *self, ssize_t ofs, int whence);  
	async_return_t (*ioctl)(struct async_task *parent, struct io_device *self, ioctl_req_t req, va_list va); 
}; 

typedef struct io_device {
	struct async_task open, close, write, read, seek, ioctl; 
	struct io_device_ops *api; 
} io_device_t; 

void io_init(struct io_device *self); 

#define IO_OPEN(io) AWAIT_TASK(io_device_t, open, io)
#define IO_CLOSE(io) AWAIT_TASK(io_device_t, close, io)
#define IO_WRITE(io, data, size) AWAIT_TASK(io_device_t, write, io, data, size)
#define IO_READ(io, data, size) AWAIT_TASK(io_device_t, read, io, data, size)
#define IO_SEEK(io, pos, from) AWAIT_TASK(io_device_t, seek, io, pos, from)
#define IO_IOCTL(io, req, ...) AWAIT_TASK(io_device_t, seek, io, req, ##__VA_ARGS__)

#define READ_INT16(buf) (((int16_t)(buf)[0] <<8) | ((int16_t)(buf)[1]))
#define READ_UINT16(buf) (((uint16_t)(buf)[0] <<8) | ((uint16_t)(buf)[1]))
#define READ_INT24(buf) (((int32_t)(buf)[0] << 16) | ((int32_t)(buf)[1] << 8) | ((int32_t)(buf)[2]))
