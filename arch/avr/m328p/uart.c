/**
	Interrupt driven fast and buffered UART for ATMega AVR

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
#include <avr/pgmspace.h>

#include <arch/soc.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <static_cbuf.h>

#define SIG_USART_RECV USART_RX_vect
#define SIG_USART_DATA USART_UDRE_vect

#define UART0_STATUS   UCSR0A
#define UART0_CONTROL  UCSR0B
#define UART0_DATA     UDR0
#define UART0_UDRIE    UDRIE0

DECLARE_STATIC_CBUF(uart0_tx_buf, char, UART_TX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart0_rx_buf, char, UART_RX_BUFFER_SIZE);

static struct {
	uint8_t error;
} uart0;

// rx int runs when new byte is received by uart hardware
ISR(USART_RX_vect) {
	uint8_t err = ( UART0_STATUS & (_BV(FE0)|_BV(DOR0)));
	uint8_t data = UDR0; //uart0_getc_direct(); 
	/*UDR0 = data; //uart0_putc_direct(data); 
	uart0_interrupt_dre_on(); 
	*/
	if(cbuf_is_full(&uart0_rx_buf)){
		err = UART_BUFFER_OVERFLOW >> 8;
	} else {
		cbuf_put(&uart0_rx_buf, data);
	}
	uart0.error = err; 
}

// data register empty int runs when hardware requests more data
ISR(USART_UDRE_vect) {
	if(cbuf_get_data_count(&uart0_tx_buf)){
		// send one byte over the wire
		uart0_putc_direct(cbuf_get(&uart0_tx_buf)); 
	} else {
		// disable interrupt
		uart0_interrupt_dre_off(); 
	}
}

void PFDECL(CONFIG_UART0_NAME, init, unsigned int baudrate)
{
	uart0_init_default(baudrate); 

	UCSR0C = (3<<UCSZ00);
}

uint16_t PFDECL(CONFIG_UART0_NAME, waiting, void){
	uart0_interrupt_rx_off(); 
	int wait = cbuf_get_data_count(&uart0_rx_buf);
	uart0_interrupt_rx_on();
	return wait; 
}

unsigned int PFDECL(CONFIG_UART0_NAME, getc, void)
{
	uart0_interrupt_rx_off(); 
	if(cbuf_is_empty(&uart0_rx_buf)) {
		uart0_interrupt_rx_on(); 
		return UART_NO_DATA;
	}
	uint8_t data = cbuf_get(&uart0_rx_buf);
	uart0_interrupt_rx_on(); 
	return data;
}

void PFDECL(CONFIG_UART0_NAME, putc, unsigned char data)
{
	// the strategy when the buffer is full is to wait for a time and then discard
	// although it could be: discard always, or wait forever
	int timeout = 2000;
	do {
		uart0_interrupt_dre_off();
		int ret = cbuf_put(&uart0_tx_buf, data);
		uart0_interrupt_dre_on();
		if(ret == -1) _delay_us(1);
		else break; 
	} while(timeout--);
}

void PFDECL(CONFIG_UART0_NAME, puts, const char *s )
{
	while (*s) 
		PFCALL(CONFIG_UART0_NAME, putc, *s++);
}

size_t PFDECL(CONFIG_UART0_NAME, write, const char *s, size_t c)
{
	size_t t = c; 
	while (c--) 
		PFCALL(CONFIG_UART0_NAME, putc, *s++);
	return t; 
}

size_t PFDECL(CONFIG_UART0_NAME, read, const char *s, size_t c)
{
	size_t t = 0; 
	while (t < c) {
		uint16_t d = PFCALL(CONFIG_UART0_NAME, getc);
		if(d == UART_NO_DATA) return t; 
		t++; 
	}
	return t; 
}

uint16_t PFDECL(CONFIG_UART0_NAME, printf, const prog_char *fmt, ...){
	char buf[uart0_tx_buf.total_size * 2]; 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf_P(buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	PFCALL(CONFIG_UART0_NAME, puts, buf);
	return n; 
}

void PFDECL(CONFIG_UART0_NAME, puts_p, const char *progmem_s )
{ 
	register char c;
	while ( (c = pgm_read_byte(progmem_s++)) ) 
		PFCALL(CONFIG_UART0_NAME, putc, c);
}

