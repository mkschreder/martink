 
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

struct block_transfer {
	uint8_t completed; 
	ssize_t address; 
	ssize_t transfered; 
	ssize_t size; 
	uint8_t *buffer; 
	io_direction_t dir; 
	block_dev_t dev; 
}; 

void blk_transfer_init(struct block_transfer *tr, 
	block_dev_t dev, uint32_t address, uint8_t *buffer, uint32_t size, io_direction_t dir); 
uint8_t blk_transfer_completed(struct block_transfer *tr); 

#ifdef __cplusplus
}
#endif
