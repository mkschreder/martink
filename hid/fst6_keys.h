#pragma once

#include <kernel/thread.h>
#include <kernel/cbuf.h>

typedef enum {
	FST6_KEY_CH1P = 0,
	FST6_KEY_CH1M = 1, 
	FST6_KEY_CH2P = 2,
	FST6_KEY_CH2M = 3,
	FST6_KEY_CH3P = 4,
	FST6_KEY_CH3M = 5,
	FST6_KEY_CH4P = 6,
	FST6_KEY_CH4M = 7,
	FST6_KEY_OK = 	8,
	FST6_KEY_CANCEL = 9,
	FST6_KEY_SWA = 10,
	FST6_KEY_SWB = 11,
	FST6_KEY_SWC = 12,
	FST6_KEY_SWD = 13,
	FST6_KEY_SELECT = 14,
	FST6_KEY_ROTA = 15,
	FST6_KEY_ROTB = 16, 
	FST6_KEYS_COUNT
} fst6_key_code_t; 

#define FST6_KEY_FLAG_UP 0x80
#define FST6_KEY_MASK 0x7f

struct fst6_keys {
	uint32_t pressed; 
	struct cbuf buffer; 
	uint8_t buffer_data[8]; 
	struct irq irq; 
	struct libk_thread thread; 
}; 

void fst6_keys_init(struct fst6_keys *self); 
int16_t fst6_keys_get(struct fst6_keys *self); 
uint8_t fst6_keys_key_down(struct fst6_keys *self, fst6_key_code_t key); 
