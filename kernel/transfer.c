
#include <arch/soc.h>

#include "transfer.h"

enum {
	BLKTR_INITIALIZED, 
	BLKTR_PROGRESS, 
	BLKTR_COMPLETED,
	BLKTR_FAILED,
	BLKTR_TIMEOUT
}; 

void blk_transfer_init(struct block_transfer *self){
	self->state = BLKTR_INITIALIZED; 
	self->offset = 0; 
	self->size = 0; 
}

int blk_transfer(struct block_transfer *self, 
	block_dev_t dev, uint8_t *buffer, ssize_t size, io_direction_t dir){
	int ret = 0; 
	switch(self->state){
		case BLKTR_INITIALIZED: {
			if(size <= 0) {
				self->state = BLKTR_COMPLETED; 
				return TR_COMPLETED; 
			}
			self->state = BLKTR_PROGRESS; 
			self->size = size; 
			// fall through
		case BLKTR_PROGRESS: 
			// start a new transfer
			if(dir == IO_READ){
				ret = blk_read(dev, buffer + (size - self->size), self->size); 
			} else if(dir == IO_WRITE){
				ret = blk_write(dev, buffer + (size - self->size), self->size); 
			}
			
			if(ret == -EOF) { // eof
				if(self->size > 0) { // bytes left
					errno = EOF; 
					self->state = BLKTR_FAILED; 
					return TR_FAILED; 
				} /*else if((blk_seek(self->dev, 0, SEEK_CUR) >= 0)){
					self->flags |= BLKTR_COMPLETED; 
					return TR_COMPLETED; 
				}*/
			} else if(ret > 0) { // bytes have been transfered
				self->size -= ret; 
				if(self->size <= 0){
					self->state = BLKTR_COMPLETED; 
					return TR_COMPLETED; 
				}
			} else {
				if(ret == -EWOULDBLOCK || ret == -EAGAIN){
					return TR_BUSY; 
				} else { // any other error
					errno = -ret; 
					self->state = BLKTR_FAILED; 
					return TR_FAILED; 
				}
			}
		} break; 
		case BLKTR_COMPLETED: 
			return TR_COMPLETED; 
		case BLKTR_FAILED: 
			return TR_FAILED; 
	}
	return TR_BUSY; 
}

uint8_t blk_transfer_busy(struct block_transfer *self){
	return self->state == BLKTR_PROGRESS; 
}

uint8_t io_begin(struct block_transfer *tr, block_dev_t dev){
	if(!blk_open(dev)) return 0; 
	blk_transfer_init(tr); 
	return 1; 
}

uint8_t io_read(struct block_transfer *tr, block_dev_t dev, ssize_t offset, uint8_t *data, ssize_t size){
	if(blk_seek(dev, offset, SEEK_SET) < 0) return 0; 
	if(blk_transfer(tr, dev, data, size, IO_READ) == TR_COMPLETED) return 1; 
	return 0; 
}

uint8_t io_write(struct block_transfer *tr, block_dev_t dev, ssize_t offset, uint8_t *data, ssize_t size){
	if(blk_seek(dev, offset, SEEK_SET) < 0) return 0; 
	if(blk_transfer(tr, dev, data, size, IO_WRITE) == TR_COMPLETED) return 1; 
	return 0; 
}

uint8_t io_end(struct block_transfer *tr, block_dev_t dev){
	if(blk_transfer_busy(tr)) return 0; 
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
