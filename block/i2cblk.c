 
#include <arch/soc.h>
#include <string.h>

#include <kernel/thread.h>
#include <kernel/dev/block.h>
#include <block/block_device.h>

#include "i2cblk.h"

/*
#define I2CBLK_FLAG_BUSY (1 << 3)
#define I2CBLK_FLAG_WRITE_ADDR (1 << 4)
#define I2CBLK_FLAG_READ (1 << 5)
#define I2CBLK_FLAG_LOCKED (1 << 6)
#define I2CBLK_FLAG_WRITE (1 << 7)
*/
#define I2CBLK_DEBUG(...) {} //printf(__VA_ARGS__) 

enum {
	I2CBLK_BAD, 
	I2CBLK_READY, 
	I2CBLK_READ_SEND_ADDRESS, 
	I2CBLK_READ, 
	I2CBLK_WRITE, 
	I2CBLK_FINISH
}; 

void i2cblk_init(struct i2c_block_device *self, block_dev_t i2c, uint8_t i2c_addr){
	self->i2c = i2c; 
	self->i2c_addr = i2c_addr; 
	self->flags = I2CBLK_IADDR8; 
	self->state = I2CBLK_READY; 
	self->cur = 0; 
	block_device_init(&self->base); 
	blk_transfer_init(&self->tr); 
	//libk_create_thread(&self->thread, _i2cblk_thread, "i2cblk"); 
}


static uint8_t _i2cblk_open(block_dev_t dev){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	//I2CBLK_DEBUG("I2CBLK: open\n"); 
	return block_device_open(&self->base); 
}

static int8_t _i2cblk_close(block_dev_t dev){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	I2CBLK_DEBUG("I2CBLK: close\n"); 
	return block_device_close(&self->base); 
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

static ssize_t _i2cblk_write(block_dev_t dev, const uint8_t *data, ssize_t data_size){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	uint8_t addr_size = 0; 
	addr_size = self->flags & I2CBLK_IADDR_BITS; 
	data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE - addr_size))?(I2C_BLOCK_BUFFER_SIZE - addr_size):data_size; 

	switch(self->state){
		case I2CBLK_READY:  
			if(!io_begin(&self->tr, self->i2c)) return -EAGAIN; 
			
			I2CBLK_DEBUG("I2CBLK: startwrite\n"); 
			
			// copy data and address into our work buffer
			_i2cblk_write_address(self); 
			
			memcpy(self->buffer + addr_size, data, data_size); 
			
			// send stop after the write
			blk_ioctl(self->i2c, I2C_SEND_STOP, 1); 
			
			self->state = I2CBLK_WRITE; 
			// fall through
		case I2CBLK_WRITE: 
			
			if(!io_write(&self->tr, self->i2c, self->i2c_addr, self->buffer, data_size + addr_size)) return -EAGAIN; 
			
			//I2CBLK_DEBUG("I2CBLK: writedone\n"); 
			//io_end(&self->tr, self->i2c); 
			
			self->state = I2CBLK_FINISH; 
			
		case I2CBLK_FINISH: 
			// need to wait until device is released
			I2CBLK_DEBUG("I2CBLK: writedone\n"); 
			if(!io_end(&self->tr, self->i2c)) return -EAGAIN; 
			self->state = I2CBLK_READY; 
			
			return data_size; 
	}
	// never get here
	return -EAGAIN; 
}

static ssize_t _i2cblk_read(block_dev_t dev, uint8_t *data, ssize_t data_size){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	uint8_t addr_size = 0; 
	
	data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE))?(I2C_BLOCK_BUFFER_SIZE):data_size; 
	
	switch(self->state){
		case I2CBLK_READY: 
			if(!io_begin(&self->tr, self->i2c)) return -EAGAIN; 
			
			I2CBLK_DEBUG("I2CBLK: startread\n"); 
			
			// prepare to write address 
			_i2cblk_write_address(self); 
			addr_size = self->flags & I2CBLK_IADDR_BITS; 
			 
			// disable sending stop after this write (so we can do repeat start)
			blk_ioctl(self->i2c, I2C_SEND_STOP, 0); 
			
			self->state = I2CBLK_READ_SEND_ADDRESS; 
			// fall through
		case I2CBLK_READ_SEND_ADDRESS: 
			addr_size = self->flags & I2CBLK_IADDR_BITS; 
			
			//I2CBLK_DEBUG("I2CBLK: sendadres\n"); 
			
			if(!io_write(&self->tr, self->i2c, self->i2c_addr, self->buffer, addr_size)) return -EAGAIN; 
			
			I2CBLK_DEBUG("I2CBLK: addrsent\n"); 
			
			self->state = I2CBLK_READ; 
			// fall through
		case I2CBLK_READ: 
			// send stop after the read
			blk_ioctl(self->i2c, I2C_SEND_STOP, 1); 
			
			if(!io_read(&self->tr, self->i2c, self->i2c_addr, self->buffer, data_size)) return -EAGAIN; 
			
			memcpy(data, self->buffer, data_size); 
			
			self->state = I2CBLK_FINISH;
			// fall through
		case I2CBLK_FINISH: 
			//I2CBLK_DEBUG("I2CBLK: readdone\n"); 
			
			if(!io_end(&self->tr, self->i2c)) return -EAGAIN; 
			I2CBLK_DEBUG("I2CBLK: readdone\n"); 
			
			self->state = I2CBLK_READY; 
			
			return data_size; 
	}
	
	return -EAGAIN; 
}
/*
static int8_t _i2cblk_get_geometry(block_dev_t dev, struct block_device_geometry *geom){
	(void)dev; 
	//struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	geom->page_size = 1; // i2c devices always byte addressable
	return 0; 
}

static uint8_t _i2cblk_get_status(block_dev_t dev, blkdev_status_t flags){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	uint8_t i2cbusy = i2cdev_status(self->i2c_id, I2CDEV_BUSY);
	uint8_t f = 0;  
	
	//if(self->flags & I2CBLK_STATUS_WRITE_PROGRESS && !i2cbusy)
	//	self->flags &= ~I2CBLK_STATUS_WRITE_PROGRESS; 
		
	if((flags & BLKDEV_BUSY) && (i2cbusy || self->flags & I2CBLK_STATUS_READ_PROGRESS))
		f |= BLKDEV_BUSY; 

	return (f & flags) == flags; 
}
*/
static ssize_t _i2cblk_seek(block_dev_t dev, ssize_t ofs, int whence){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
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
	return self->cur; 
}


static int16_t _i2cblk_ioctl(block_dev_t dev, ioctl_req_t req, ...){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	switch(req) {
		case I2CBLK_SET_AW: 
			I2CBLK_DEBUG("I2CBLK: aw not implemented!\n"); 
			break; 
		default: 
			return -EINVAL; 
	}
	return -1; 
}

block_dev_t i2cblk_get_interface(struct i2c_block_device *self){
	static struct block_device_ops _if;
	static struct block_device_ops *i = 0; 
	
	if(!i){
		_if.open = _i2cblk_open; 
		_if.close = _i2cblk_close; 
		_if.write = _i2cblk_write; 
		_if.read = _i2cblk_read; 
		_if.ioctl = _i2cblk_ioctl; 
		_if.seek = _i2cblk_seek; 
		i = &_if; 
	}
	self->api = i; 
	return &self->api; 
}


