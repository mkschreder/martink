#include <arch/soc.h>
#include "pipe.h"


static uint16_t _pipe_in_putc(serial_dev_t self, uint8_t ch){
	struct pipe *p = container_of(self, struct pipe, input); 
	cbuf_put(&p->txbuf, ch); 
	return SERIAL_NO_DATA; 
}

static uint16_t _pipe_in_getc(serial_dev_t self) {
	struct pipe *p = container_of(self, struct pipe, input); 
	return cbuf_get(&p->rxbuf); 
}

static size_t _pipe_in_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_pipe_in_putc(self, *data++); 
	}
	return size; 
}

static size_t _pipe_in_getn(serial_dev_t self, uint8_t *data, size_t sz){
	struct pipe *p = container_of(self, struct pipe, input); 
	size_t count = sz; 
	if(cbuf_get_waiting(&p->rxbuf) < sz) return 0; 
	while(sz--){
		*data++ = _pipe_in_getc(self); 
	}
	return count; 
}

static size_t _pipe_in_waiting(serial_dev_t self){
	struct pipe *p = container_of(self, struct pipe, input); 
	return cbuf_get_waiting(&p->rxbuf); 
}

static int16_t _pipe_begin(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}

static int16_t _pipe_end(serial_dev_t self){
	// do nothing (but may be useful for interrupt driven version) 
	(void)(self); 
	return 0; 
}


static uint16_t _pipe_out_putc(serial_dev_t self, uint8_t ch){
	struct pipe *p = container_of(self, struct pipe, output); 
	cbuf_put(&p->rxbuf, ch); 
	return SERIAL_NO_DATA; 
}

static uint16_t _pipe_out_getc(serial_dev_t self) {
	struct pipe *p = container_of(self, struct pipe, output); 
	return cbuf_get(&p->txbuf); 
}

static size_t _pipe_out_putn(serial_dev_t self, const uint8_t *data, size_t sz){
	size_t size = sz;
	while(sz--){
		_pipe_out_putc(self, *data++); 
	}
	return size; 
}

static size_t _pipe_out_getn(serial_dev_t self, uint8_t *data, size_t sz){
	struct pipe *p = container_of(self, struct pipe, output); 
	size_t count = sz; 
	if(cbuf_get_waiting(&p->txbuf) < sz) return 0; 
	while(sz--){
		*data++ = _pipe_out_getc(self); 
	}
	return count; 
}

static size_t _pipe_out_waiting(serial_dev_t self){
	struct pipe *p = container_of(self, struct pipe, output); 
	return cbuf_get_waiting(&p->txbuf); 
}

struct cbuf *pipe_get_rx_cbuf(struct pipe *self){
	return &self->rxbuf; 
}

struct cbuf *pipe_get_tx_cbuf(struct pipe *self){
	return &self->txbuf; 
}

void pipe_init(struct pipe *self, uint8_t *rxbuf, uint16_t rxsize, uint8_t *txbuf, uint16_t txsize){
	cbuf_init(&self->rxbuf, rxbuf, rxsize); 
	cbuf_init(&self->txbuf, txbuf, txsize); 
	
	static struct serial_if _in_if; 
	static struct serial_if *in_if = 0; 
	static struct serial_if _out_if; 
	static struct serial_if *out_if = 0; 
	if(!in_if){
		_in_if = (struct serial_if) {
			.put = _pipe_in_putc,
			.get = _pipe_in_getc,
			.putn = _pipe_in_putn,
			.getn = _pipe_in_getn,
			.begin = _pipe_begin,
			.end = _pipe_end,
			.waiting = _pipe_in_waiting
		}; 
		_out_if = (struct serial_if) {
			.put = _pipe_out_putc,
			.get = _pipe_out_getc,
			.putn = _pipe_out_putn,
			.getn = _pipe_out_getn,
			.begin = _pipe_begin,
			.end = _pipe_end,
			.waiting = _pipe_out_waiting
		}; 
		in_if = &_in_if; 
		out_if = &_out_if; 
	}
	self->input = in_if; 
	self->output = out_if; 
}

serial_dev_t pipe_get_input(struct pipe *self){
	return &self->input; 
}

serial_dev_t pipe_get_output(struct pipe *self){
	return &self->output; 
}
