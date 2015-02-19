#pragma once 

#include <inttypes.h>
#include <stdint.h>

#define CBUF_NO_DATA 0xff00

struct cbuf {
	uint8_t *buffer; 
	int16_t size; 
	int16_t head, tail; 
}; 

#ifdef __cplusplus
extern "C" {
#endif

void cbuf_init(struct cbuf *self, uint8_t *buffer, uint16_t size); 
uint16_t cbuf_get_waiting(struct cbuf *self); 
uint16_t cbuf_get_free(struct cbuf *self); 
void cbuf_clear(struct cbuf *self); 
int8_t cbuf_is_empty(struct cbuf *self); 
int8_t cbuf_is_full(struct cbuf *self); 
uint16_t cbuf_get(struct cbuf *self); 
int8_t cbuf_put(struct cbuf *self, uint8_t data);
uint16_t cbuf_getn(struct cbuf *self, uint8_t *data, uint16_t size); 
uint16_t cbuf_putn(struct cbuf *self, const uint8_t *data, uint16_t size); 
uint8_t cbuf_peek(struct cbuf *self, uint16_t pos);

#ifdef __cplusplus
}
#endif

