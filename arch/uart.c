/**
	Portable uart driver code

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

static struct {
	uint8_t error;
} uart0;

DECLARE_STATIC_CBUF(uart0_tx_buf, uint8_t, UART_TX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart0_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);

DECLARE_UART0_RX_INTERRUPT(uart0_rx_buf, uart0.error); 
DECLARE_UART0_TX_INTERRUPT()
DECLARE_UART0_DRE_INTERRUPT(uart0_tx_buf); 

void PFDECL(CONFIG_UART0_NAME, init, unsigned int baudrate) {
	uart0_init_default(baudrate); 
}

size_t PFDECL(CONFIG_UART0_NAME, waiting, void){
	uart0_interrupt_rx_off(); 
	int wait = cbuf_get_data_count(&uart0_rx_buf);
	uart0_interrupt_rx_on();
	return wait; 
}

void PFDECL(CONFIG_UART0_NAME, flush, void){
	uint16_t timeout = 2000; 
	while(timeout--){
		uart0_interrupt_dre_off(); 
		int empty = cbuf_is_empty(&uart0_tx_buf);
		uart0_interrupt_dre_on();
		if(empty) break; 
		time_static_delay_us(1); 
	}
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

uint16_t PFDECL(CONFIG_UART0_NAME, printf, const char *fmt, ...){
	char buf[uart0_tx_buf.total_size * 2]; 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf(buf, sizeof(buf)-1, fmt, vl); 
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

