
/*
 Base class for a block device. Basic open / close state management
 */
 
#pragma once

#include <kernel/dev/block.h>
#include <kernel/thread.h>
#include <stdarg.h>

typedef struct io_device *io_dev_t; 
typedef ssize_t io_result_t; 

struct io_device; 

struct io_device_ops {
	async_return_t (*open)(io_result_t *ret, struct async_task *parent, struct io_device *self); 
	async_return_t (*close)(io_result_t *ret, struct async_task *parent, struct io_device *self); 
	async_return_t (*write)(io_result_t *ret, struct async_task *parent, struct io_device *self, const uint8_t *data, ssize_t data_size); 
	async_return_t (*read)(io_result_t *ret, struct async_task *parent, struct io_device *self, uint8_t *data, ssize_t data_size); 
	async_return_t (*seek)(io_result_t *ret, struct async_task *parent, struct io_device *self, ssize_t ofs, int whence);  
	async_return_t (*ioctl)(io_result_t *ret, struct async_task *parent, struct io_device *self, ioctl_req_t req, va_list va); 
}; 

typedef struct io_device {
	struct async_task vopen, vclose, vwrite, vread, vseek, vioctl; 
	struct io_device_ops *api; 
} io_device_t; 

void io_init(struct io_device *self); 

ASYNC_PROTOTYPE(ssize_t, io_device_t, open); 
ASYNC_PROTOTYPE(ssize_t, io_device_t, close); 
ASYNC_PROTOTYPE(ssize_t, io_device_t, write, const uint8_t *data, ssize_t size); 
ASYNC_PROTOTYPE(ssize_t, io_device_t, read, uint8_t *data, ssize_t size); 
ASYNC_PROTOTYPE(ssize_t, io_device_t, seek, ssize_t ofs, int whence); 
ASYNC_PROTOTYPE(ssize_t, io_device_t, ioctl, ioctl_req_t req, ...);

#define IO_OPEN(io) 								AWAIT_TASK(io_result_t, io_device_t, open, io)
#define IO_CLOSE(io) 								AWAIT_TASK(io_result_t, io_device_t, close, io)
#define IO_WRITE(io, data, size) 		AWAIT_TASK(io_result_t, io_device_t, write, io, data, size)
#define IO_READ(io, data, size) 		AWAIT_TASK(io_result_t, io_device_t, read, io, data, size)
#define IO_SEEK(io, pos, from) 			AWAIT_TASK(io_result_t, io_device_t, seek, io, pos, from)
#define IO_IOCTL(io, req, ...) 			AWAIT_TASK(io_result_t, io_device_t, ioctl, io, req, ##__VA_ARGS__)

static inline int16_t READ_INT16(const uint8_t *buf) { return (((int16_t)(buf)[0] <<8) | ((int16_t)(buf)[1])); }
static inline uint16_t READ_UINT16(const uint8_t *buf) { return (((uint16_t)(buf)[0] <<8) | ((uint16_t)(buf)[1])); }
static inline int32_t READ_INT24(const uint8_t *buf) { return (((int32_t)(buf)[0] << 16) | ((int32_t)(buf)[1] << 8) | ((int32_t)(buf)[2])); }
/*
#define READ_INT16(buf) (((int16_t)(buf)[0] <<8) | ((int16_t)(buf)[1]))
#define READ_UINT16(buf) (((uint16_t)(buf)[0] <<8) | ((uint16_t)(buf)[1]))
#define READ_INT24(buf) (((int32_t)(buf)[0] << 16) | ((int32_t)(buf)[1] << 8) | ((int32_t)(buf)[2]))
*/
