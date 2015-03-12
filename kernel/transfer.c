
#include <arch/soc.h>

#include "transfer.h"

#define BLKTR_COMPLETED (1 << 0)
#define BLKTR_FAILED (1 << 1)
#define BLKTR_TIMEOUT (1 << 2)
#define BLKTR_INITIALIZED (1 << 7)

void blk_transfer_init(struct block_transfer *tr){
	tr->flags = BLKTR_INITIALIZED; 
	tr->offset = 0; 
	tr->size = 0; 
	tr->buffer = 0; 
	tr->dev = 0; 
}

void blk_transfer_start(struct block_transfer *tr, 
	block_dev_t dev, uint8_t *buffer, ssize_t size, io_direction_t dir){
	tr->flags = 0; 
	tr->offset = 0; 
	tr->size = size; 
	tr->buffer = buffer; 
	tr->dir = dir; 
	tr->dev = dev; 
	
	if(size <= 0){
		tr->flags |= TR_COMPLETED; 
	} else {
		// kick off the transfer right away
		blk_transfer_result(tr);
	}
}

blk_transfer_result_t blk_transfer_result(struct block_transfer *tr){
	int ret = 0; 
	if(tr->flags & (BLKTR_COMPLETED)) return TR_COMPLETED; 
	else if(tr->flags & BLKTR_FAILED) return TR_FAILED; 
	
	if(tr->size <= 0){
		tr->flags |= BLKTR_COMPLETED; 
		return TR_COMPLETED; 
	}
	
	if(tr->dir == IO_READ){
		// if not all bytes have been transfered then send more
		ret = blk_read(tr->dev, tr->buffer, tr->size); 
	} else if(tr->dir == IO_WRITE){
		ret = blk_write(tr->dev, tr->buffer, tr->size); 
	}
	
	if(ret == 0) { // eof
		if(tr->size > 0) { // bytes left
			tr->flags |= BLKTR_FAILED; 
			errno = EPIPE; 
			return TR_FAILED; 
		} else if((blk_seek(tr->dev, 0, SEEK_CUR) >= 0)){
			tr->flags |= BLKTR_COMPLETED; 
			return TR_COMPLETED; 
		}
	} else if(ret > 0) { // bytes have been read
		tr->buffer += ret; 
		tr->size -= ret; 
		if(tr->size <= 0){
			tr->flags |= BLKTR_COMPLETED; 
			return TR_COMPLETED; 
		}
	} else {
		if(ret == -EWOULDBLOCK || ret == -EAGAIN){
			return TR_BUSY; 
		} else {
			errno = -ret; 
			tr->flags |= BLKTR_FAILED; 
			return TR_FAILED; 
		}
	}
	
	return TR_BUSY; 
}

void blk_transfer_cancel(struct block_transfer *tr){
	tr->flags |= BLKTR_COMPLETED; 
	tr->size = 0; 
	tr->offset = 0; 
}
