 
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
	//block_dev_t dev; // target device
	ssize_t offset;  // device offset
	//uint8_t *buffer; // current buffer position
	ssize_t size; 	 // size left to process
	//ssize_t transfered; 
	//io_direction_t dir; 
	uint8_t state; 
}; 
/*
void blk_transfer_init(struct block_transfer *tr); 
void blk_transfer_cancel(struct block_transfer *tr); 
void blk_transfer_start(struct block_transfer *tr, 
	block_dev_t dev, uint8_t *buffer, ssize_t size, io_direction_t dir); 
blk_transfer_result_t blk_transfer_result(struct block_transfer *tr); 
*/
void blk_transfer_init(struct block_transfer *tr); 
int blk_transfer(struct block_transfer *tr, block_dev_t dev, uint8_t *data, ssize_t size, io_direction_t dir); 
uint8_t blk_transfer_busy(struct block_transfer *self); 

uint8_t io_begin(struct block_transfer *tr, block_dev_t dev);
uint8_t io_read(struct block_transfer *tr, block_dev_t dev, ssize_t offset, uint8_t *data, ssize_t size);
uint8_t io_write(struct block_transfer *tr, block_dev_t dev, ssize_t offset, uint8_t *data, ssize_t size); 
uint8_t io_end(struct block_transfer *tr, block_dev_t dev);


#define IO_SLEEP(time, delay) do { time = timestamp_from_now_us(delay); \
		PT_WAIT_UNTIL(thr, timestamp_expired(time)); } while(0); 

#define IO_BEGIN(pt, io, dev) PT_WAIT_UNTIL(pt, io_begin(io, dev))
#define IO_READ(pt, io, dev, addr, data, size) PT_WAIT_UNTIL(pt, io_read(io, dev, addr, data, size))
#define IO_WRITE(pt, io, dev, addr, data, size) PT_WAIT_UNTIL(pt, io_write(io, dev, addr, data, size))
#define IO_END(pt, tr, dev) PT_WAIT_UNTIL(pt, io_end(tr, dev))

#define READ_INT16(buf) (((int16_t)(buf)[0] <<8) | ((int16_t)(buf)[1]))
#define READ_UINT16(buf) (((uint16_t)(buf)[0] <<8) | ((uint16_t)(buf)[1]))
#define READ_INT24(buf) (((int32_t)(buf)[0] << 16) | ((int32_t)(buf)[1] << 8) | ((int32_t)(buf)[2]))

#ifdef __cplusplus
}
#endif
