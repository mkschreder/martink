#pragma once 

#include <inttypes.h>
#include <stdint.h>

#define CBUF_NO_DATA 0xff00

struct cbuf {
	uint8_t *buffer; 
	int16_t size; 
	int16_t head, tail; 
	uint32_t lock; 
}; 

#ifdef __cplusplus
extern "C" {
#endif

void cbuf_init(struct cbuf *self, uint8_t *buffer, uint16_t size); 
uint16_t cbuf_get_waiting(struct cbuf *self); 
uint16_t cbuf_get_waiting_isr(struct cbuf *self); 
uint16_t cbuf_get_free(struct cbuf *self); 
void cbuf_clear(struct cbuf *self); 
uint8_t cbuf_is_empty(struct cbuf *self); 
uint8_t cbuf_is_full(struct cbuf *self); 
uint8_t cbuf_is_full_isr(struct cbuf *self); 
uint16_t cbuf_get(struct cbuf *self); 
uint16_t cbuf_get_isr(struct cbuf *self); 
uint8_t cbuf_put(struct cbuf *self, uint8_t data);
uint8_t cbuf_put_isr(struct cbuf *self, uint8_t data);
uint16_t cbuf_getn(struct cbuf *self, uint8_t *data, uint16_t size); 
uint16_t cbuf_putn(struct cbuf *self, const uint8_t *data, uint16_t size); 
uint16_t cbuf_peek(struct cbuf *self, uint16_t pos);

#ifdef __cplusplus
}
#endif

