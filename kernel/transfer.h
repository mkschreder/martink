 
#pragma once

#include <inttypes.h>

#include "dev/block.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	IO_WRITE, 
	IO_READ
} io_direction_t; 

typedef enum {
	TR_BUSY = 0, 
	TR_COMPLETED = 1, 
	TR_FAILED = 2, 
	TR_TIMEOUT = 3
} blk_transfer_result_t; 

struct block_transfer {
	block_dev_t dev; // target device
	ssize_t offset;  // device offset
	uint8_t *buffer; // current buffer position
	ssize_t size; 	 // size left to process
	//ssize_t transfered; 
	io_direction_t dir; 
	uint8_t flags; 
}; 

void blk_transfer_init(struct block_transfer *tr); 
void blk_transfer_cancel(struct block_transfer *tr); 
void blk_transfer_start(struct block_transfer *tr, 
	block_dev_t dev, uint8_t *buffer, ssize_t size, io_direction_t dir); 
blk_transfer_result_t blk_transfer_result(struct block_transfer *tr); 

#ifdef __cplusplus
}
#endif
