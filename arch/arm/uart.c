/**
	This file is part of martink project.

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

#include <inttypes.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <arch/soc.h>

#include "uart.h"

#define UART_TX_BUFFER_SIZE 32


void UART1_Handler(void) {
   /*uint32_t dw_status = USART_(UART1);

   if(dw_status & UART_SR_RXRDY) {
      uint8_t received_byte;
      //uart_read(UART1, &received_byte);
      //uart_write(UART1, received_byte);
   }*/
}

void UART0_Handler(void) {
   /*uint32_t dw_status = USART_(UART1);

   if(dw_status & UART_SR_RXRDY) {
      uint8_t received_byte;
      //uart_read(UART1, &received_byte);
      //uart_write(UART1, received_byte);
   }*/
}

void PFDECL(CONFIG_UART0_NAME, init, uint32_t baud){
	pmc_enable_periph_clk(ID_PIOA);
  pmc_enable_periph_clk(ID_USART0);
	PIO_Configure(PIOA,PIO_PERIPH_A,(PIO_PA10A_RXD0|PIO_PA11A_TXD0),PIO_DEFAULT);
	USART_Configure(USART0, USART_MODE_ASYNCHRONOUS, baud, SystemCoreClock);
	USART_DisableIt(USART0,0xFFFFFFFF);
	USART_SetTransmitterEnabled(USART0, 1);
	USART_SetReceiverEnabled(USART0, 1);
	USART_EnableIt(USART0,UART_IER_ENDRX);
}

size_t PFDECL(CONFIG_UART0_NAME, write, const uint8_t *data, size_t max_size){
	// wait for previous op to complete
	while(!(USART_GetStatus(USART0) & US_CSR_TXRDY)); 
	for(int c = 0; c < max_size; c++){
		USART_PutChar(USART0, data[c]); 
	}
	//return USART_WriteBuffer(USART0, data, max_size);
}

size_t PFDECL(CONFIG_UART0_NAME, read, uint8_t *data, size_t max_size){
	if(!(USART_GetStatus(USART0) & US_CSR_RXRDY)) return 0;  
	return USART_ReadBuffer(USART0, data, max_size); 
}

uint16_t PFDECL(CONFIG_UART0_NAME, getc, struct serial_interface *self){
	return USART_GetChar(USART0); 
}

uint16_t PFDECL(CONFIG_UART0_NAME, putc, struct serial_interface *self, uint8_t ch){
	if(!(USART_GetStatus(USART0) & US_CSR_TXRDY)) return 0; 
	USART_PutChar(USART0, ch); 
	return 1; 
}

size_t PFDECL(CONFIG_UART0_NAME, puts, const char *str){
	uint16_t n = 0; 
	while(*str){
		PFCALL(CONFIG_UART0_NAME, putc, 0, *str); 
		n++; str++; 
	}
	return n; 
}

uint16_t PFDECL(CONFIG_UART0_NAME, printf, const char *fmt, ...){
	static uint8_t buf[UART_TX_BUFFER_SIZE * 2]; 
	//memcpy(buf, "Hello WOrld!", 10); 
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf((char*)buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	
	PFCALL(CONFIG_UART0_NAME, write, buf, n); 
	
	return n; 
}
