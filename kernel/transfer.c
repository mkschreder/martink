
#include <arch/soc.h>

#include "transfer.h"

#define TR_DEBUG(...) {} //printf(__VA_ARGS__) 

enum {
	BLKTR_READY, 
	BLKTR_PROGRESS, 
	//BLKTR_COMPLETED,
	//BLKTR_FAILED,
	//BLKTR_TIMEOUT
}; 

void blk_transfer_init(struct block_transfer *self){
	self->state = BLKTR_READY; 
	self->size = 0; 
}

int blk_transfer(struct block_transfer *self, 
	block_dev_t dev, ssize_t offset, uint8_t *buffer, ssize_t size, io_direction_t dir){
	int ret = 0; 
	switch(self->state){
		case BLKTR_READY: {
			TR_DEBUG("TR: start\n"); 
			//if(!blk_open(dev)) return -EAGAIN; 
			
			if(size <= 0) {
				self->size = 0; 
				return TR_COMPLETED; 
			}
			self->state = BLKTR_PROGRESS; 
			self->size = size; 
			
			// fall through
		case BLKTR_PROGRESS: 
			// keep the transfer going
			if(blk_seek(dev, offset + (size - self->size), SEEK_SET) < 0) return -EAGAIN; 
			//TR_DEBUG("TR: seek to %d \n", offset + (size - self->size)); 
			
			if(dir == IO_READ){
				ret = blk_read(dev, buffer + (size - self->size), self->size); 
			} else if(dir == IO_WRITE){
				ret = blk_write(dev, buffer + (size - self->size), self->size); 
			}
			//TR_DEBUG("TR: ret %d: %s\n", ret, strerror(-ret)); 
			
			if(ret < 0){
				return TR_BUSY; 
			} else if(ret > 0){
				self->size -= ret; 
				TR_DEBUG("TR: transfered %d bytes, left %d\n", ret, self->size); 
				if(self->size <= 0){ 
					//self->state = BLKTR_COMPLETED; 
					self->size = 0; 
					self->state = BLKTR_READY; 
					return TR_COMPLETED; 
				}
			} else { // 0
				printf("TR: ret == 0: should not happen!\n"); 
				//while(1); 
			}
		} break; 
		/*case BLKTR_COMPLETED: 
			return TR_COMPLETED; 
		case BLKTR_FAILED: 
			return TR_FAILED; */
	}
	return TR_BUSY; 
}

uint8_t blk_transfer_busy(struct block_transfer *self){
	return self->state == BLKTR_PROGRESS; 
}

uint8_t io_begin(struct block_transfer *tr, block_dev_t dev){
	if(!blk_open(dev)) return 0; 
	(void)dev; 
	blk_transfer_init(tr); 
	return 1; 
}

uint8_t io_read(struct block_transfer *tr, block_dev_t dev, ssize_t offset, uint8_t *data, ssize_t size){
	//if(blk_seek(dev, offset, SEEK_SET) < 0) return 0; 
	//if(!blk_transfer_busy(tr)) blk_transfer_init(tr);  
	if(blk_transfer(tr, dev, offset, data, size, IO_READ) == TR_BUSY) return 0; 
	//blk_transfer_init(tr);  
	return 1; 
}

uint8_t io_write(struct block_transfer *tr, block_dev_t dev, ssize_t offset, uint8_t *data, ssize_t size){
	//if(blk_seek(dev, offset, SEEK_SET) < 0) return 0; 
	//if(!blk_transfer_busy(tr)) blk_transfer_init(tr);  
	if(blk_transfer(tr, dev, offset, data, size, IO_WRITE)  == TR_BUSY) return 0; 
	//blk_transfer_init(tr); 
	return 1; 
}

uint8_t io_end(struct block_transfer *tr, block_dev_t dev){
	(void)dev; (void)tr; 
	//if(blk_transfer_busy(tr)) return 0; 
	if(blk_close(dev) < 0) return 0; 
	return 1;
}
/*
void blk_transfer_start(struct block_transfer *tr, 
	block_dev_t dev, uint8_t *buffer, ssize_t size, io_direction_t dir){
	self->flags = 0; 
	self->offset = 0; 
	self->size = size; 
	self->buffer = buffer; 
	self->dir = dir; 
	self->dev = dev; 
	
	if(size <= 0){
		self->flags |= TR_COMPLETED; 
	} else {
		// kick off the transfer right away
		blk_transfer_result(tr);
	}
}

blk_transfer_result_t blk_transfer_result(struct block_transfer *tr){
	int ret = 0; 
	if(self->flags & (BLKTR_COMPLETED)) return TR_COMPLETED; 
	else if(self->flags & BLKTR_FAILED) return TR_FAILED; 
	
	if(self->size <= 0){
		self->flags |= BLKTR_COMPLETED; 
		return TR_COMPLETED; 
	}
	
	if(self->dir == IO_READ){
		// if not all bytes have been transfered then send more
		ret = blk_read(self->dev, self->buffer, self->size); 
	} else if(self->dir == IO_WRITE){
		ret = blk_write(self->dev, self->buffer, self->size); 
	}
	
	if(ret == 0) { // eof
		if(self->size > 0) { // bytes left
			self->flags |= BLKTR_FAILED; 
			errno = EPIPE; 
			return TR_FAILED; 
		} else if((blk_seek(self->dev, 0, SEEK_CUR) >= 0)){
			self->flags |= BLKTR_COMPLETED; 
			return TR_COMPLETED; 
		}
	} else if(ret > 0) { // bytes have been read
		self->buffer += ret; 
		self->size -= ret; 
		if(self->size <= 0){
			self->flags |= BLKTR_COMPLETED; 
			return TR_COMPLETED; 
		}
	} else {
		if(ret == -EWOULDBLOCK || ret == -EAGAIN){
			return TR_BUSY; 
		} else {
			errno = -ret; 
			self->flags |= BLKTR_FAILED; 
			return TR_FAILED; 
		}
	}
	
	return TR_BUSY; 
}

void blk_transfer_cancel(struct block_transfer *tr){
	self->flags |= BLKTR_COMPLETED; 
	self->size = 0; 
	self->offset = 0; 
}*/
