 
#include <arch/soc.h>
#include <string.h>

#include <kernel/thread.h>
#include <kernel/dev/block.h>

#include "i2cblk.h"

/*
#define I2CBLK_FLAG_BUSY (1 << 3)
#define I2CBLK_FLAG_WRITE_ADDR (1 << 4)
#define I2CBLK_FLAG_READ (1 << 5)
#define I2CBLK_FLAG_LOCKED (1 << 6)
#define I2CBLK_FLAG_WRITE (1 << 7)
*/
#define I2CBLK_DEBUG(...) {} //printf(__VA_ARGS__) 

#define I2CBLK_IO_TIMEOUT 500000

enum {
	I2CBLK_BAD, 
	I2CBLK_READY, 
	I2CBLK_READ_SEND_ADDRESS, 
	I2CBLK_READ, 
	I2CBLK_WRITE, 
	I2CBLK_FINISH
}; 

void i2cblk_init(struct i2c_block_device *self, io_dev_t i2c, uint8_t i2c_addr){
	self->i2c = i2c; 
	self->i2c_addr = i2c_addr; 
	self->flags = I2CBLK_IADDR8; 
	self->cur = 0; 
	io_init(&self->io); 
	//libk_create_thread(&self->thread, _i2cblk_thread, "i2cblk"); 
}


static PT_THREAD(_i2cblk_open(struct pt *pt, struct io_device *dev)){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, io); 
	
	PT_BEGIN(pt); 
	
	self->cur = 0; 
	
	PT_END(pt); 
}

static PT_THREAD(_i2cblk_close(struct pt *pt, struct io_device *dev)){
	//struct i2c_block_device *self = container_of(dev, struct i2c_block_device, io); 
	(void)dev; 
	I2CBLK_DEBUG("I2CBLK: close\n"); 
	
	PT_BEGIN(pt); 
	PT_END(pt); 
}


static void _i2cblk_write_address(struct i2c_block_device *dev){
	if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR8) {
		dev->buffer[0] = dev->cur; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR16) {
		dev->buffer[0] = dev->cur >> 1; 
		dev->buffer[1] = dev->cur; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR32) {
		dev->buffer[0] = dev->cur >> 3; 
		dev->buffer[1] = dev->cur >> 2; 
		dev->buffer[2] = dev->cur >> 1; 
		dev->buffer[3] = dev->cur; 
	}
}

static PT_THREAD(_i2cblk_write(struct pt *pt, struct io_device *dev, const uint8_t *data, ssize_t data_size)){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, io); 
	
	uint8_t addr_size = 0; 
	addr_size = self->flags & I2CBLK_IADDR_BITS; 
	data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE - addr_size))?(I2C_BLOCK_BUFFER_SIZE - addr_size):data_size; 

	PT_BEGIN(pt); 
	
	PT_ASYNC_BEGIN(pt, self->i2c, I2CBLK_IO_TIMEOUT); 
	
	I2CBLK_DEBUG("I2CBLK: startwrite\n"); 
	
	while(1){
		// copy data and address into our work buffer
		_i2cblk_write_address(self); 
		
		memcpy(self->buffer + addr_size, data, data_size); 
	
		// send stop after the write
		PT_ASYNC_IOCTL(pt, self->i2c, I2CBLK_IO_TIMEOUT, I2C_SEND_STOP, 1); 
		
		PT_ASYNC_SEEK(pt, self->i2c, I2CBLK_IO_TIMEOUT, self->i2c_addr, SEEK_SET); 
		PT_ASYNC_WRITE(pt, self->i2c, I2CBLK_IO_TIMEOUT, self->buffer, data_size + addr_size); 
		
		self->cur += data_size; 
		
		if(_io_progress(&self->io, data_size) == 0){
			break; 
		}
		
		PT_YIELD(pt); 
	}
	
	I2CBLK_DEBUG("I2CBLK: writedone\n"); 
			
	PT_ASYNC_END(pt, self->i2c, I2CBLK_IO_TIMEOUT); 
	
	PT_END(pt);  
}

static PT_THREAD(_i2cblk_read(struct pt *pt, struct io_device *dev, uint8_t *data, ssize_t data_size)){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, io); 
	
	uint8_t addr_size = self->flags & I2CBLK_IADDR_BITS; 
	data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE))?(I2C_BLOCK_BUFFER_SIZE):data_size; 
	
	PT_BEGIN(pt); 
	
	while(1){
		PT_ASYNC_BEGIN(pt, self->i2c, I2CBLK_IO_TIMEOUT); 
	
		I2CBLK_DEBUG("I2CBLK: startread\n"); 
	
		// prepare to write address 
		_i2cblk_write_address(self); 
		 
		// disable sending stop after this write (so we can do repeat start)
		PT_ASYNC_IOCTL(pt, self->i2c, I2CBLK_IO_TIMEOUT, I2C_SEND_STOP, 0); 
		
		PT_ASYNC_SEEK(pt, self->i2c, I2CBLK_IO_TIMEOUT, self->i2c_addr, SEEK_SET); 
		PT_ASYNC_WRITE(pt, self->i2c, I2CBLK_IO_TIMEOUT, self->buffer, addr_size); 
				
		I2CBLK_DEBUG("I2CBLK: addrsent\n"); 
		
		// read the data and send stop after the read
		PT_ASYNC_IOCTL(pt, self->i2c, I2CBLK_IO_TIMEOUT, I2C_SEND_STOP, 1); 
		PT_ASYNC_READ(pt, self->i2c, I2CBLK_IO_TIMEOUT, self->buffer, data_size); 
		
		PT_ASYNC_END(pt, self->i2c, I2CBLK_IO_TIMEOUT); 
		
		memcpy(data, self->buffer, data_size); 
		self->cur += data_size; 
		
		if(_io_progress(&self->io, data_size) == 0){
			break; 
		}
		PT_YIELD(pt); 
	}
	
	I2CBLK_DEBUG("I2CBLK: readdone\n"); 
	
	PT_END(pt); 
}

static PT_THREAD(_i2cblk_seek(struct pt *pt, struct io_device *dev, ssize_t ofs, int whence)){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, io); 
	
	switch(whence){
		case SEEK_SET: self->cur = ofs; break; 
		case SEEK_END: {
			if((self->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR8)
				self->cur = (0xff + ofs) & 0xff; 
			else if((self->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR16)
				self->cur = (0xffff + ofs) & 0xffff; 
			else if((self->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR32)
				self->cur = (0x7fffffff + ofs) & 0x7fffffff; 
		} break; 
		case SEEK_CUR: self->cur += ofs; break; 
	}
	
	PT_BEGIN(pt); 
	PT_END(pt); 
}


static PT_THREAD(_i2cblk_ioctl(struct pt *pt, struct io_device *dev, ioctl_req_t req, va_list vl)){
	//struct i2c_block_device *self = container_of(dev, struct i2c_block_device, io); 
	(void)vl; 
	(void)dev; 
	switch(req) {
		case I2CBLK_SET_AW: 
			I2CBLK_DEBUG("I2CBLK: aw not implemented!\n"); 
			break; 
	}
	
	PT_BEGIN(pt); 
	PT_END(pt); 
}

io_dev_t i2cblk_get_interface(struct i2c_block_device *self){
	static struct io_device_ops _if;
	
	if(!self->io.api){
		_if.open = _i2cblk_open; 
		_if.close = _i2cblk_close; 
		_if.write = _i2cblk_write; 
		_if.read = _i2cblk_read; 
		_if.ioctl = _i2cblk_ioctl; 
		_if.seek = _i2cblk_seek; 
		self->io.api = &_if; 
	}
	
	return &self->io; 
}


