#pragma once 

#include "cbuf.h"
#include <arch/interface.h>

struct pipe {
	struct cbuf rxbuf; // input <- output
	struct cbuf txbuf; // input -> output
	struct serial_if *input, *output; 
};

#ifdef __cplusplus
extern "C" {
#endif

void pipe_init(struct pipe *self, uint8_t *rxbuf, uint16_t rxsize, uint8_t *txbuf, uint16_t txsize); 
struct cbuf *pipe_get_rx_cbuf(struct pipe *self); 
struct cbuf *pipe_get_tx_cbuf(struct pipe *self); 
serial_dev_t pipe_get_input(struct pipe *self); 
serial_dev_t pipe_get_output(struct pipe *self); 

#ifdef __cplusplus
}
#endif
