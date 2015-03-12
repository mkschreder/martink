 
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
	//libk_create_thread(&self->thread, _i2cblk_thread, "i2cblk"); 
}

/*
static uint8_t _i2cblk_open(block_dev_t dev){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, base.api); 
	return block_device_open(&self->base); 
}

static int8_t _i2cblk_close(block_dev_t dev){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, base.api); 
	return block_device_close(&self->base); 
}
*/

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

	switch(self->state){
		case I2CBLK_READY:  
			if(!io_begin(&self->tr, self->i2c)) return -EAGAIN; 
			
			// copy data and address into our work buffer
			_i2cblk_write_address(self); 
			
			addr_size = self->flags & I2CBLK_IADDR_BITS;  
			data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE - addr_size))?(I2C_BLOCK_BUFFER_SIZE - addr_size):data_size; 
			
			memcpy(self->buffer + addr_size, data, data_size); 
			
			// send stop after the write
			blk_ioctl(self->i2c, I2C_SEND_STOP, 1); 
			
			self->state = I2CBLK_WRITE; 
			// fall through
		case I2CBLK_WRITE: 
			addr_size = self->flags & I2CBLK_IADDR_BITS; 
			data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE - addr_size))?(I2C_BLOCK_BUFFER_SIZE - addr_size):data_size; 
			
			if(!io_write(&self->tr, self->i2c, self->i2c_addr, self->buffer, data_size + addr_size)) return -EAGAIN; 
			
			self->state = I2CBLK_FINISH; 
			// will return number of bytes once after first trying to release the device
			return data_size + addr_size; 
		case I2CBLK_FINISH: 
			// need to wait until device is released
			if(!io_end(&self->tr, self->i2c)) return -EAGAIN; 
			self->state = I2CBLK_READY; 
	}
	// never get here
	return -EAGAIN; 
}

static ssize_t _i2cblk_read(block_dev_t dev, uint8_t *data, ssize_t data_size){
	struct i2c_block_device *self = container_of(dev, struct i2c_block_device, api); 
	if(!block_device_can_access(&self->base)) return -EACCES; 
	
	uint8_t addr_size = 0; 

	switch(self->state){
		case I2CBLK_READY: 
			if(!io_begin(&self->tr, self->i2c)) return -EAGAIN; 
			
			// prepare to write address 
			_i2cblk_write_address(self); 
			addr_size = self->flags & I2CBLK_IADDR_BITS;  
			// disable sending stop after this write (so we can do repeat start)
			blk_ioctl(self->i2c, I2C_SEND_STOP, 0); 
			
			self->state = I2CBLK_READ_SEND_ADDRESS; 
			// fall through
		case I2CBLK_READ_SEND_ADDRESS: 
			addr_size = self->flags & I2CBLK_IADDR_BITS; 
			if(!io_write(&self->tr, self->i2c, self->i2c_addr, self->buffer, addr_size)) return -EAGAIN; 
			self->state = I2CBLK_READ; 
			// fall through
		case I2CBLK_READ: 
			data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE))?(I2C_BLOCK_BUFFER_SIZE):data_size; 
		
			if(!io_read(&self->tr, self->i2c, self->i2c_addr, self->buffer, data_size)) return -EAGAIN; 
			
			memcpy(data, self->buffer, data_size); 
			
			self->state = I2CBLK_FINISH;
			
			// return successfully transfered bytes 
			return data_size; 
			// fall through
		case I2CBLK_FINISH: 
			if(!io_end(&self->tr, self->i2c)) return -EAGAIN; 
			self->state = I2CBLK_READY; 
	}
	/*
			ddr_size = _i2cblk_prep_address(self); 
			data_size = (data_size > (I2C_BLOCK_BUFFER_SIZE))?(I2C_BLOCK_BUFFER_SIZE):data_size; 
			
			if(!blk_open(self->i2c)) return -EAGAIN; 
			blk_ioctl(self->i2c, I2C_SEND_STOP, 0); 
			blk_seek(self->i2c, self->i2c_addr, SEEK_SET); 
			blk_transfer_start(&self->tr, self->i2c, self->buffer, addr_size, IO_WRITE); 
			self->flags |= I2CBLK_FLAG_READ | I2CBLK_FLAG_WRITE_ADDR; 
	}
	
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
	}*/
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
		struct block_device_ops *ops = *block_device_get_interface(&self->base); 
		memcpy(&_if, ops, sizeof(struct block_device_ops)); 
		_if = (struct block_device_ops) {
			//.open = _i2cblk_open, 
			//.close = _i2cblk_close, 
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


