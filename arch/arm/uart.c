#include <inttypes.h>
#include <stdarg.h>
#include <string.h>

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
	memcpy(buf, "Hello World!", 10); 
	
	uint16_t n; 
	va_list vl; 
	va_start(vl, fmt);
	n = 10; //vsnprintf(buf, sizeof(buf)-1, fmt, vl); 
	va_end(vl);
	
	USART_WriteBuffer(USART0, buf, n);
	return n; 
}
