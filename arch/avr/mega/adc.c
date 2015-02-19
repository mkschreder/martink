/**
	Fast ADC implementation for ATmega using macros

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include <arch/soc.h>

#include "adc.h"


uint8_t _adc_mode = ADC_MODE_AUTOMATIC;

//#if defined(CONFIG_ADC_MODE_AUTOMATIC)
//uint16_t _adc_values[8];
/*
LIST_HEAD(_connections); 

struct adc_ev_header {
	uint8_t id; 
	uint16_t size; 
}; 

struct adc_ev_conv_completed {
	struct adc_ev_header header; 
	timestamp_t time; 
	uint8_t chan; 
	uint16_t value; 
}; 

void adc_connect(struct adc_connection *con, uint8_t *rxbuf, uint8_t *txbuf, uint8_t size, int8_t (*on_event)(struct adc_connection *self, uint16_t ev)){
	INIT_LIST_HEAD(&con->list); 
	pipe_init(&con->pipe, rxbuf, size, txbuf, size); 
	con->on_event = on_event; 
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		list_add_tail(&con->list, &_connections); 
	}
}
int8_t adc_read_ev_conv_completed(struct adc_connection *self, 
	timestamp_t *time, uint8_t *chan, uint16_t *value){
	struct cbuf *device = pipe_get_rx_cbuf(&self->pipe); 
	
	uint16_t waiting = cbuf_get_waiting(device); 
	if(waiting >= sizeof(struct adc_ev_conv_completed) && cbuf_peek(device, 0) == ADC_EV_CONV_COMPLETED){
		struct adc_ev_conv_completed ev; 
		cbuf_getn(device, (uint8_t*)&ev, sizeof(struct adc_ev_conv_completed)); 
		*time = ev.time; 
		*chan = ev.chan; 
		*value = ev.value; 
		return 0; 
	} 
	return -1; 
}
*/
/*
int8_t adc_msg_value_parse(struct adc_connection *con, struct adc_msg_value *msg){
	struct cbuf *device = pipe_get_rx_cbuf(&con->pipe); 
	printf("pack: peek: %x, wait: %d, free: %d\n", cbuf_peek(device, 0), cbuf_get_waiting(device	), cbuf_get_free(device)); 
	//for(volatile uint32_t c = 0; c < 100000; c++); 
	if(cbuf_get_waiting(device) >= sizeof(struct adc_msg_value) && cbuf_peek(device, 0) == ADC_EV_CONV_COMPLETED){
		uint8_t buf[4]; 
		cbuf_getn(device, buf, 4); 
		msg->chan = buf[1]; 
		msg->val = (buf[2] << 8) | buf[3]; 
		return 0; 
	} else if(cbuf_get_waiting(device) > 0){
		printf("%x ", cbuf_get(device)); 
	}
	return -1; 
}*/
/*
static void _adc_pack_conversion_response(struct adc_connection *con, timestamp_t time, uint8_t chan, uint16_t value){
	struct cbuf *user = pipe_get_rx_cbuf(&con->pipe); 
	if(cbuf_get_free(user) < sizeof(struct adc_ev_conv_completed)) return; 
	struct adc_ev_conv_completed ev = {
		.header = {
			.id = ADC_EV_CONV_COMPLETED, 
			.size = sizeof(struct adc_ev_conv_completed) - sizeof(struct adc_ev_header), 
		}, 
		.time = time, 
		.chan = chan, 
		.value = value
	}; 
	cbuf_putn(user, (uint8_t*)&ev, sizeof(struct adc_ev_conv_completed)); 
}

void adc_process_events(void){
	struct list_head *pos, *n; 
	list_for_each_safe(pos, n, &_connections){
		struct adc_connection *con = container_of(pos, struct adc_connection, list); 
		struct cbuf *device = pipe_get_rx_cbuf(&con->pipe); 
		uint16_t waiting = cbuf_get_waiting(device); 
		if(con->on_event(con, cbuf_peek(device, 0)) == -1 && waiting > sizeof(struct adc_ev_header)){
			struct adc_ev_header h; 
			cbuf_getn(device, (uint8_t*)&h, sizeof(struct adc_ev_header)); 
			// if wrong size then discard the buffer
			if(h.size > waiting) cbuf_clear(device); 
			// otherwise remove the number of bytes needed
			else for(uint16_t c = 0; c < h.size; c++) cbuf_get(device); 
		}
	}
}*/

static volatile uint16_t *_adc_data = 0; 
static volatile uint8_t _adc_status = 0; 

uint8_t adc_busy(void){
	return _adc_status == 1; 
}

void adc_start_conversion(uint8_t channel, volatile uint16_t *value){
	if(_adc_status != 0) return; 
	_adc_data = value; 
	_adc_status = 1; 
	adc0_set_channel(channel & 0x07); 
	adc0_start_conversion(); 
}

ISR(ADC_vect){
	if(!_adc_data) return; 
	*_adc_data = ADC; 
	_adc_data = 0; 
	_adc_status = 0; 
	/*_adc_values[adc0_get_channel() & 0x07] = adc;
	if(_adc_mode == ADC_MODE_AUTOMATIC){
		adc0_set_channel((adc0_get_channel() + 1) & 0x07); 
		adc0_start_conversion(); 
	}*/
}
//#endif
