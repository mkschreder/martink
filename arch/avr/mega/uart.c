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

#if defined(CONFIG_BUFFERED_UART)
	/** Size of the circular receive buffer, must be power of 2 */
	#ifndef CONFIG_UART0_TX_BUF_SIZE
	#define CONFIG_UART0_TX_BUF_SIZE 32
	#endif
	/** Size of the circular transmit buffer, must be power of 2 */
	#ifndef CONFIG_UART0_RX_BUF_SIZE
	#define CONFIG_UART0_RX_BUF_SIZE 32
	#endif

	#define UART_TX_BUFFER_SIZE CONFIG_UART0_TX_BUF_SIZE
	#define UART_RX_BUFFER_SIZE CONFIG_UART0_RX_BUF_SIZE

	DECLARE_STATIC_CBUF(uart0_tx_buf, uint8_t, UART_TX_BUFFER_SIZE);
	DECLARE_STATIC_CBUF(uart0_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);

	static struct {
		uint8_t error;
	} uart0;

	ISR(USART_RX_vect) { 
		uint8_t err = ( UART0_STATUS & (_BV(FE0)|_BV(DOR0))); 
		uint8_t data = UDR0; 
		if(cbuf_is_full(&uart0_rx_buf)){ 
			err = SERIAL_BUFFER_FULL >> 8; 
		} else { 
			cbuf_put(&uart0_rx_buf, data); 
		} 
		uart0.error = err; 
	}

	ISR(USART_TX_vect) {

	}

	ISR(USART_UDRE_vect) {
		if(cbuf_get_data_count(&uart0_tx_buf)){
			uart0_putc_direct(cbuf_get(&uart0_tx_buf)); 
		} else {
			uart0_interrupt_dre_off(); 
		}
	}

	size_t uart0_waiting(void){
		uart0_interrupt_rx_off(); 
		int wait = cbuf_get_data_count(&uart0_rx_buf);
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
			if(ret == -1) _delay_us(1);
			else return 0; 
		} while(timeout--);
		return SERIAL_BUFFER_FULL; 
	}

#endif
