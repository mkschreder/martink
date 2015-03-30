 
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

void i2cblk_init(struct i2c_block_device *dev, io_dev_t i2c, uint8_t i2c_addr, uint16_t block_size, uint8_t flags){
	dev->i2c = i2c; 
	dev->i2c_addr = i2c_addr; 
	dev->block_size = block_size; 
	dev->flags = flags; 
	dev->addr = 0; 
	dev->offset = 0; 
	io_init(&dev->io); 
	ASYNC_MUTEX_INIT(dev->buffer_lock, 1); 
	ASYNC_MUTEX_INIT(dev->lock, 1); 
	//libk_create_thread(&dev->thread, __io_device_t_vthread, "i2cblk"); 
}


static ASYNC(io_result_t, io_device_t, vopen){
	struct i2c_block_device *dev = container_of(__self, struct i2c_block_device, io); 
	
	ASYNC_BEGIN(); 
	
	ASYNC_MUTEX_LOCK(dev->lock); 
	
	dev->addr = 0; 
	//dev->flags = I2CBLK_IADDR8; 
	dev->offset = 0; 
	
	I2CBLK_DEBUG("I2CBLK: open\n"); 
	
	ASYNC_END(0); 
}

static ASYNC(io_result_t, io_device_t, vclose){
	struct i2c_block_device *dev = container_of(__self, struct i2c_block_device, io); 

	ASYNC_BEGIN(); 
	ASYNC_MUTEX_UNLOCK(dev->lock); 
	
	I2CBLK_DEBUG("I2CBLK: close\n"); 
	
	ASYNC_END(0);
}


static void _write_address(struct i2c_block_device *dev){
	if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR8) {
		dev->buffer[0] = dev->addr; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR16) {
		dev->buffer[0] = dev->addr >> 8; 
		dev->buffer[1] = dev->addr; 
	} else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR32) {
		dev->buffer[0] = dev->addr >> 24; 
		dev->buffer[1] = dev->addr >> 16; 
		dev->buffer[2] = dev->addr >> 8; 
		dev->buffer[3] = dev->addr; 
	}
}

static ASYNC(io_result_t, io_device_t, vwrite, const uint8_t *data, ssize_t size){
	struct i2c_block_device *dev = container_of(__self, struct i2c_block_device, io); 
	
	uint8_t addr_size = dev->flags & I2CBLK_IADDR_BITS; 
	ssize_t tr_size = ((size - dev->offset) > (ssize_t)dev->block_size)?(ssize_t)dev->block_size:(size - dev->offset); 
	// extra clamp to the buffer size because we can't send more data than we have space in buffer
	tr_size = (tr_size > I2C_BLOCK_BUFFER_SIZE)?I2C_BLOCK_BUFFER_SIZE:tr_size; 
	
	ASYNC_BEGIN(); 
	
		ASYNC_MUTEX_LOCK(dev->buffer_lock); 
		
		I2CBLK_DEBUG("I2CBLK: writing %d bytes\n", size); 
		
		
		while(dev->offset < size){
			
			// copy data and address into our work buffer
			_write_address(dev); 
			
			memcpy(dev->buffer + addr_size, data, tr_size); 
			
			I2CBLK_DEBUG("I2CBLK: writing %d bytes using %d bytes address\n", tr_size, addr_size); 
			
			IO_OPEN(dev->i2c); 
		
			// send stop after the write
			IO_IOCTL(dev->i2c, I2C_SEND_STOP, 1); 
			
			IO_SEEK(dev->i2c, dev->i2c_addr, SEEK_SET); 
			
			IO_WRITE(dev->i2c, dev->buffer, tr_size + addr_size); 
			IO_CLOSE(dev->i2c); 
		
			I2CBLK_DEBUG("I2CBLK: wrote %d bytes\n", tr_size); 
		
			dev->offset += tr_size; 
			dev->addr += tr_size; 
			
			ASYNC_YIELD(); 
		}
		
		I2CBLK_DEBUG("I2CBLK: writedone\n"); 
		
		// always reset the offset at the end
		dev->offset = 0; 
		
		ASYNC_MUTEX_UNLOCK(dev->buffer_lock); 

	ASYNC_END(dev->offset);  
}

static ASYNC(io_result_t, io_device_t, vread, uint8_t *data, ssize_t size){
	struct i2c_block_device *dev = container_of(__self, struct i2c_block_device, io); 
	(void)data; 
	
	uint8_t addr_size = dev->flags & I2CBLK_IADDR_BITS; 
	ssize_t tr_size = ((size - dev->offset) > (ssize_t)dev->block_size)?(ssize_t)dev->block_size:(size - dev->offset); 
	tr_size = (size > I2C_BLOCK_BUFFER_SIZE)?I2C_BLOCK_BUFFER_SIZE:size; 
	
	(void)addr_size; 
	
	ASYNC_BEGIN(); 
		
		ASYNC_MUTEX_LOCK(dev->buffer_lock); 
		
		//I2CBLK_DEBUG("I2CBLK: reading %d bytes, using %d byte address\n", size, addr_size); 
		
		while(dev->offset < size){
			I2CBLK_DEBUG("I2CBLK: reading %d byte block to %d\n", tr_size, dev->offset); 
			
			
			// prepare to write address 
			_write_address(dev); 
			IO_OPEN(dev->i2c); 
		
			IO_SEEK(dev->i2c, dev->i2c_addr, SEEK_SET); 
			
			IO_IOCTL(dev->i2c, I2C_SEND_STOP, (dev->flags & I2CBLK_SEND_STOP_AFTER_ADDR)?1:0); 
			
			IO_WRITE(dev->i2c, dev->buffer, addr_size); 
			
			// read the data and send stop after the read
			IO_IOCTL(dev->i2c, I2C_SEND_STOP, 1); 
			
			// read one block at a time
			IO_READ(dev->i2c, data + dev->offset, tr_size); 
			
			IO_CLOSE(dev->i2c); 
		
			
			I2CBLK_DEBUG("I2CBLK: read %d bytes to %d\n", tr_size, dev->offset); 
			
			dev->offset += tr_size; 
			dev->addr += tr_size; 
			
			ASYNC_YIELD(); 
		}
		
		I2CBLK_DEBUG("I2CBLK: readdone\n"); 
		
		// always reset the offset at the end
		dev->offset = 0; 
	
		ASYNC_MUTEX_UNLOCK(dev->buffer_lock); 
	
	ASYNC_END(dev->offset); 
}

static ASYNC(io_result_t, io_device_t, vseek, ssize_t ofs, int whence){
	struct i2c_block_device *dev = container_of(__self, struct i2c_block_device, io); 
	
	ASYNC_BEGIN(); 
	
	ASYNC_MUTEX_LOCK(dev->buffer_lock); 
	
	if(whence == SEEK_SET) dev->addr = ofs; 
	else if(whence == SEEK_END) {
		if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR8)
			dev->addr = (0xff + ofs) & 0xff; 
		else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR16)
			dev->addr = (0xffff + ofs) & 0xffff; 
		else if((dev->flags & I2CBLK_IADDR_BITS) == I2CBLK_IADDR32)
			dev->addr = (0x7fffffff + ofs) & 0x7fffffff; 
	} else if(whence == SEEK_CUR) dev->addr += ofs;
	
	I2CBLK_DEBUG("I2CBLK: seek %x\n", (uint16_t)dev->addr); 
	
	ASYNC_MUTEX_UNLOCK(dev->buffer_lock); 
	
	ASYNC_END(dev->addr); 
}


static ASYNC(io_result_t, io_device_t, vioctl, ioctl_req_t req, va_list vl){
	struct i2c_block_device *dev = container_of(__self, struct i2c_block_device, io); 

	ASYNC_BEGIN(); 
	
	ASYNC_MUTEX_LOCK(dev->buffer_lock); 
	
	if(req == I2CBLK_SEND_STOP_AFTER_ADDR){
		uint8_t en = va_arg(vl, int); 
		if(en)
			dev->flags |= I2CBLK_SEND_STOP_AFTER_ADDR; 
		else
			dev->flags &= ~I2CBLK_SEND_STOP_AFTER_ADDR; 
	}
	
	ASYNC_MUTEX_UNLOCK(dev->buffer_lock); 
	
	ASYNC_END(0); 
}

io_dev_t i2cblk_get_interface(struct i2c_block_device *self){
	static struct io_device_ops _if;
	
	if(!self->io.api){
		_if.open = __io_device_t_vopen__; 
		_if.close = __io_device_t_vclose__; 
		_if.write = __io_device_t_vwrite__; 
		_if.read = __io_device_t_vread__; 
		_if.ioctl = __io_device_t_vioctl__; 
		_if.seek = __io_device_t_vseek__; 
		self->io.api = &_if; 
	}
	
	return &self->io; 
}

/*
typedef uint8_t io_event_t; 

static void i2cblk_i2c_isr(io_device_t *source, io_event_t ev){
	// figure out the source of the event
	struct i2c_block_device *self = container_of(source, struct i2c_block_device, i2c); 
	SM(&self->sm, IDLE){
		SM_STATE(IDLE){
			switch(ev){
				case IO_DEVICE_OPENED: {
					if(GET_BIT(self->flags, I2CBLK_READ)){
						io_start_read(self->i2c, self->data, self->size); 
						SM_ENTER(READ); 
					}
				}
			}
		}
		STATE(READ){
			switch(ev){
				case IO_RX_DATA_READY: {
					
				}
			}
		}
	}
	return SM_RESULT(); 
}

ssize_t i2cblk_read(struct i2c_block_device *self, uint8_t *data, ssize_t size){
	if(i2cblk_busy(self)) return -EBUSY; 
	SET_BIT(self->flags, I2CBLK_READ); 
	self->data = data; 
	self->size = size; 
	io_start_open(self->i2c, &self->irq); 
	return 0; 
}
*/
