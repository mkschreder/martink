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

void __uart0_init__(uint32_t baud){
	pmc_enable_periph_clk(ID_PIOA);
  pmc_enable_periph_clk(ID_USART0);
	PIO_Configure(PIOA,PIO_PERIPH_A,(PIO_PA10A_RXD0|PIO_PA11A_TXD0),PIO_DEFAULT);
	USART_Configure(USART0, USART_MODE_ASYNCHRONOUS, baud, SystemCoreClock);
	USART_DisableIt(USART0,0xFFFFFFFF);
	USART_SetTransmitterEnabled(USART0, 1);
	USART_SetReceiverEnabled(USART0, 1);
	USART_EnableIt(USART0,UART_IER_ENDRX);
}

uint16_t __uart0_printf__(const char *fmt, ...){
	char buf[UART_TX_BUFFER_SIZE * 2]; 
	//memcpy(buf, "Hello WOrld!", 10); 
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = vsnprintf(buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	
	USART_WriteBuffer(USART0, buf, n);
	return n; 
}
