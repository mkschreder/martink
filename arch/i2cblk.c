 
#include <arch/soc.h>
#include <string.h>

#include <kernel/thread.h>
#include <kernel/dev/block.h>

#include "i2cblk.h"


#define I2CBLK_FLAG_BUSY (1 << 3)
#define I2CBLK_FLAG_WRITE_ADDR (1 << 4)
#define I2CBLK_FLAG_READ (1 << 5)
#define I2CBLK_FLAG_LOCKED (1 << 6)
#define I2CBLK_FLAG_WRITE (1 << 7)

#define I2CBLK_DEBUG(...) {} //printf(__VA_ARGS__) 

void i2cblk_init(struct i2c_block_device *self, block_dev_t i2c, uint8_t i2c_addr){
	self->i2c = i2c; 
	self->i2c_addr = i2c_addr; 
	self->flags = I2CBLK_IADDR8; 
	self->cur = 0; 
	//libk_create_thread(&self->thread, _i2cblk_thread, "i2cblk"); 
}

static uint8_t _i2cblk_open(block_dev_t self){
	struct i2c_block_device *at = container_of(self, struct i2c_block_device, api); 
	if((at->flags & I2CBLK_FLAG_LOCKED)) return 0; 
	at->flags |= I2CBLK_FLAG_LOCKED; 
	at->user_thread = libk_current_thread(); 
	return 1; 
}

static int8_t _i2cblk_close(block_dev_t self){
	struct i2c_block_device *at = container_of(self, struct i2c_block_device, api); 
	if(at->user_thread != libk_current_thread()) return -EACCES; 
	at->flags &= ~I2CBLK_FLAG_LOCKED; 
	at->user_thread = 0; 
	return 0;
}

static uint8_t _i2cblk_prep_address(struct i2c_block_device *dev){
	uint8_t addr_size = 0; 
	if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR0) {
		addr_size = 0; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR8) {
		dev->buffer[0] = dev->cur; 
		addr_size = 1; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR16) {
		dev->buffer[0] = dev->cur >> 1; 
		dev->buffer[1] = dev->cur; 
		addr_size = 2; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR32) {
		dev->buffer[0] = dev->cur >> 3; 
		dev->buffer[1] = dev->cur >> 2; 
		dev->buffer[2] = dev->cur >> 1; 
		dev->buffer[3] = dev->cur; 
		addr_size = 4; 
	}
	return addr_size; 
}

static ssize_t _i2cblk_write(block_dev_t dev, const uint8_t *data, ssize_t data_size){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	// check whether a transfer is in progress
	if(blk_transfer_result(&self->tr) == TR_BUSY) return -EWOULDBLOCK; 
	
	I2CBLK_DEBUG("I2CBLK: write\n"); 
	
	if(!(self->flags & I2CBLK_FLAG_WRITE)){
		// start new write
		if(!blk_open(self->i2c)) return -EAGAIN; 
		
		uint8_t addr_size = _i2cblk_prep_address(self); 
		data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE - addr_size))?(I2C_BLOCK_BUFFER_SIZE - addr_size):data_size; 
		
		memcpy(self->buffer + addr_size, data, data_size); 
		
		// set i2c address
		blk_seek(self->i2c, self->i2c_addr, SEEK_SET); 
		blk_ioctl(self->i2c, I2C_SEND_STOP, 1); 
		
		self->flags |= I2CBLK_FLAG_WRITE; 
		
		// start the transfer
		blk_transfer_start(&self->tr, self->i2c, self->buffer, data_size + addr_size, IO_WRITE); 
		
		return -EAGAIN; 
	} else {
		int ret = blk_transfer_result(&self->tr); 
		if(ret == TR_BUSY) return -EAGAIN; 
		if(ret <= 0) return ret; 
		self->flags &= ~I2CBLK_FLAG_WRITE; 
		blk_close(self->i2c); 
		return data_size; 
	}
	return -EAGAIN; 
}

static ssize_t _i2cblk_read(block_dev_t dev, uint8_t *data, ssize_t data_size){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
	if(blk_transfer_result(&self->tr) == TR_BUSY) {
		//I2CBLK_DEBUG("I2CBLK: readbusy\n"); 
		return -EWOULDBLOCK; 
	}
	
	
	// read requires us to first do a write with the address we want to read and then to do a read
	// so if we have not started a read yet then we set up a write and start it. 
	if(!(self->flags & I2CBLK_FLAG_READ)){
		I2CBLK_DEBUG("I2CBLK: write address\n"); 
		uint8_t addr_size = _i2cblk_prep_address(self); 
		data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE))?(I2C_BLOCK_BUFFER_SIZE):data_size; 
		
		if(!blk_open(self->i2c)) return -EAGAIN; 
		blk_ioctl(self->i2c, I2C_SEND_STOP, 0); 
		blk_seek(self->i2c, self->i2c_addr, SEEK_SET); 
		blk_transfer_start(&self->tr, self->i2c, self->buffer, addr_size, IO_WRITE); 
		self->flags |= I2CBLK_FLAG_READ | I2CBLK_FLAG_WRITE_ADDR; 
	} else { // we only get here after the 
		// check transfer status
		int ret = blk_transfer_result(&self->tr); 
		if(self->flags & I2CBLK_FLAG_WRITE_ADDR){
			if(ret == TR_COMPLETED){
				// start the read
				I2CBLK_DEBUG("I2CBLK: startread\n"); 
				
				blk_ioctl(self->i2c, I2C_SEND_STOP, 1); 
				blk_seek(self->i2c, self->i2c_addr, SEEK_SET); 
				memset(self->buffer, 0, sizeof(self->buffer)); 
				blk_transfer_start(&self->tr, self->i2c, self->buffer, data_size, IO_READ); 
				self->flags &= ~I2CBLK_FLAG_WRITE_ADDR; 
				return -EWOULDBLOCK; 
			} else if(ret == TR_FAILED){
				I2CBLK_DEBUG("I2CBLK: failed\n"); 
				self->flags &= ~(I2CBLK_FLAG_WRITE_ADDR | I2CBLK_FLAG_READ); 
				return -EIO; 
			} else {
				I2CBLK_DEBUG("I2CBLK: writeaddrbusy\n"); 
				return -EAGAIN; 
			}
		} else {
			if(ret == TR_COMPLETED){
				I2CBLK_DEBUG("I2CBLK: completed\n"); 
				blk_close(self->i2c); 
				self->flags &= ~I2CBLK_FLAG_READ; // reset the read flag
				memcpy(data, self->buffer, data_size); 
				return data_size; 
			} else if(ret == TR_FAILED){
				I2CBLK_DEBUG("I2CBLK: failed\n"); 
				blk_close(self->i2c); 
				blk_transfer_cancel(&self->tr); 
				self->flags &= ~I2CBLK_FLAG_READ; // reset the read flag
				return -EINVAL; 
			} else {
				I2CBLK_DEBUG("I2CBLK: again\n"); 
				return -EAGAIN; 
			}
		}
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
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
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
	if(self->user_thread != libk_current_thread()) return -EACCES; 
	
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
	static struct block_device _if;
	static struct block_device *i = 0; 
	
	if(!i){
		_if = (struct block_device) {
			.open = _i2cblk_open, 
			.close = _i2cblk_close, 
			.write = _i2cblk_write,
			.read = _i2cblk_read,
			.ioctl = _i2cblk_ioctl, 
			.seek = _i2cblk_seek, 
			//.get_geometry = _i2cblk_get_geometry,
			//.get_status = _i2cblk_get_status
		}; 
		i = &_if; 
	}
	self->api = i; 
	return &self->api; 
}


