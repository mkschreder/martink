/**
	This file is part of martink kernel library

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

#include <arch/soc.h>
#include <kernel/cbuf.h>
//#if defined(CONFIG_BUFFERED_UART)

static struct cbuf uart0_tx_buf, uart0_rx_buf; 

//DECLARE_STATIC_CBUF(uart0_tx_buf, uint8_t, UART_TX_BUFFER_SIZE);
//DECLARE_STATIC_CBUF(uart0_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);

static struct {
	uint8_t error;
} uart0;

ISR(USART_RX_vect) { 
	uint8_t err = ( UART0_STATUS & (_BV(FE0)|_BV(DOR0))); 
	uint8_t data = UDR0; 
	if(cbuf_is_full_isr(&uart0_rx_buf)){ 
		err = SERIAL_BUFFER_FULL >> 8; 
	} else { 
		cbuf_put_isr(&uart0_rx_buf, data); 
	} 
	uart0.error = err; 
}

ISR(USART_TX_vect) {

}

ISR(USART_UDRE_vect) {
	if(cbuf_get_waiting_isr(&uart0_tx_buf)){
		uart0_putc_direct(cbuf_get_isr(&uart0_tx_buf)); 
	} else {
		uart0_interrupt_dre_off(); 
	}
}

size_t uart0_waiting(void){
	uart0_interrupt_rx_off(); 
	int wait = cbuf_get_waiting(&uart0_rx_buf);
	uart0_interrupt_rx_on();
	return wait; 
}

void uart0_flush(void){
	uint16_t timeout = 2000; 
	while(timeout--){
		uart0_interrupt_dre_off(); 
		int empty = cbuf_is_empty(&uart0_tx_buf);
		uart0_interrupt_dre_on();
		if(empty) break; 
		_delay_us(1); 
	}
}

uint16_t uart0_getc(void){
	uart0_interrupt_rx_off(); 
	if(cbuf_is_empty(&uart0_rx_buf)) {
		uart0_interrupt_rx_on(); 
		return SERIAL_NO_DATA;
	}
	uint8_t data = cbuf_get(&uart0_rx_buf);
	uart0_interrupt_rx_on(); 
	return data;
}

uint16_t uart0_putc(uint8_t data){
	// the strategy when the buffer is full is to wait for a time and then discard
	// although it could be: discard always, or wait forever
	int timeout = 2000;
	do {
		uart0_interrupt_dre_off();
		int ret = cbuf_put(&uart0_tx_buf, data);
		uart0_interrupt_dre_on();
		if(ret == 0) _delay_us(1);
		else return 0; 
	} while(timeout--);
	return SERIAL_BUFFER_FULL; 
}
//#endif

int8_t 		uart_init(uint8_t dev_id, uint32_t baud, uint8_t *tx_buffer, uint8_t tx_size, uint8_t *rx_buffer, uint8_t rx_size){
	cbuf_init(&uart0_tx_buf, tx_buffer, tx_size); 
	cbuf_init(&uart0_rx_buf, rx_buffer, rx_size); 
	switch(dev_id){
		case 0: uart0_init_default(baud); break; 
		default: return -1; 
	}
	return 0; 
}

uint16_t 	uart_getc(uint8_t dev_id){
	switch(dev_id){
		case 0: return uart0_getc(); 
		default: return SERIAL_NO_DATA; 
	}
}

int8_t 		uart_putc(uint8_t dev_id, uint8_t ch){
	switch(dev_id){
		case 0: uart0_putc(ch); return 0; 
		default: return -1; 
	}
}

uint16_t 	uart_waiting(uint8_t dev_id){
	switch(dev_id){
		case 0: return uart0_waiting(); 
		default: return 0; 
	}
}
