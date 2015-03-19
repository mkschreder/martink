
/*
 Base class for a block device. Basic open / close state management
 */
 
#pragma once

#include <kernel/dev/block.h>
#include <kernel/thread.h>
#include <stdarg.h>

typedef struct io_device *io_dev_t; 

struct io_device_ops {
	async_return_t (*open)(struct pt *pt, io_dev_t self); 
	async_return_t (*close)(struct pt *pt, io_dev_t self); 
	async_return_t (*write)(struct pt *pt, io_dev_t self, const uint8_t *data, ssize_t data_size); 
	async_return_t (*read)(struct pt *pt, io_dev_t self, uint8_t *data, ssize_t data_size); 
	async_return_t (*seek)(struct pt *pt, io_dev_t self, ssize_t ofs, int whence);  
	async_return_t (*ioctl)(struct pt *pt, io_dev_t self, ioctl_req_t req, va_list va); 
}; 

struct io_device {
	struct pt thread; 
	struct pt *user_thread; 
	ssize_t size; // bytes left to transfer
	timestamp_t time; 
	uint8_t status; 
	int16_t error; 
	timestamp_t timeout; 
	struct io_device_ops *api; 
}; 

void io_init(struct io_device *self); 
ssize_t _io_progress(struct io_device *self, ssize_t processed); 

uint8_t io_open(io_dev_t dev, timestamp_t tout); 
uint8_t io_close(io_dev_t dev); 
uint8_t io_write(io_dev_t dev, const uint8_t *data, ssize_t size); 
uint8_t io_read(io_dev_t dev, uint8_t *data, ssize_t size); 
uint8_t io_seek(io_dev_t dev, ssize_t ofs, int whence); 
uint8_t io_ioctl(io_dev_t dev, ioctl_req_t req, ...); 

ssize_t io_error(io_dev_t dev); 

#define PT_ASYNC_BEGIN(pt, bd, timeout) PT_WAIT_UNTIL(pt, io_open(bd, timeout))
#define PT_ASYNC_WRITE(pt, bd, timeout, buffer, size) PT_WAIT_UNTIL(pt, io_write(bd, buffer, size))
#define PT_ASYNC_READ(pt, bd, timeout, buffer, size) PT_WAIT_UNTIL(pt, io_read(bd, buffer, size))
#define PT_ASYNC_SEEK(pt, bd, timeout, pos, whence) PT_WAIT_UNTIL(pt, io_seek(bd, pos, whence))
#define PT_ASYNC_IOCTL(pt, bd, timeout, ...) PT_WAIT_UNTIL(pt, io_ioctl(bd, __VA_ARGS__))
#define PT_ASYNC_END(pt, bd, timeout) PT_WAIT_UNTIL(pt, io_close(bd))

#define READ_INT16(buf) (((int16_t)(buf)[0] <<8) | ((int16_t)(buf)[1]))
#define READ_UINT16(buf) (((uint16_t)(buf)[0] <<8) | ((uint16_t)(buf)[1]))
#define READ_INT24(buf) (((int32_t)(buf)[0] << 16) | ((int32_t)(buf)[1] << 8) | ((int32_t)(buf)[2]))
