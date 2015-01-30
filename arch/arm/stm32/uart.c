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

#include "f10x/stm32f10x.h"
#include "f10x/stm32f10x_usart.h"
#include "f10x/stm32f10x_gpio.h"
#include "f10x/stm32f10x_rcc.h"

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

#define gpioa_clock_en() 

static void _uart_init(USART_TypeDef *uart, uint32_t baud){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |
												 RCC_APB2Periph_GPIOA |
												 RCC_APB2Periph_AFIO, ENABLE);
	
	USART_InitTypeDef usartConfig;

	usartConfig.USART_BaudRate = 38400;
	usartConfig.USART_WordLength = USART_WordLength_8b;
	usartConfig.USART_StopBits = USART_StopBits_1;
	usartConfig.USART_Parity = USART_Parity_No;
	usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usartConfig.USART_HardwareFlowControl =
			 USART_HardwareFlowControl_None;
			 
	USART_Init(uart, &usartConfig);
	USART_Cmd(uart, ENABLE);

	RCC->APB2ENR |= 0
			// Turn on USART1
			| RCC_APB2ENR_USART1EN
			// Turn on IO Port A
			| RCC_APB2ENR_IOPAEN
			// Turn on the alternate function block
			| RCC_APB2ENR_AFIOEN;
			
	uart->BRR = F_CPU/baud;
}

void uart0_init_default(uint32_t baud){
	_uart_init(USART1, baud); 
	
	GPIO_InitTypeDef gpioConfig;

	//PA9 = USART1.TX => Alternative Function Output
	gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioConfig.GPIO_Pin = GPIO_Pin_9;
	gpioConfig.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &gpioConfig);

	//PA10 = USART1.RX => Input
	gpioConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpioConfig.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &gpioConfig);
	
	RCC->APB2ENR |= 0
		// Turn on USART1
		| RCC_APB2ENR_USART1EN
		// Turn on IO Port A
		| RCC_APB2ENR_IOPAEN
		// Turn on the alternate function block
		| RCC_APB2ENR_AFIOEN;
			
	/*GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin =
					GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	*/
	
	// set up clocks for UART functions
	
	/*// set up pin B0 as output (push-pull)
	GPIOA->CRH = 0xBBBBBBBB;

	// Put PB0 into push pull 50 MHz mode
	GPIOB->CRL = 0x03;
	USART1->BRR = 72000000/38400;
	// Enable the UART, TX, and RX circuit
	USART1->CR1 |= USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;*/
}

/*
size_t uart0_write(const uint8_t *data, size_t max_size){
	for(int c = 0; c < max_size; c++){
		uart0_putc(data[c]); 
	}
	return max_size; 
	//return USART_WriteBuffer(USART0, data, max_size);
}

size_t uart0_read(uint8_t *data, size_t max_size){
	//if(!(USART_GetStatus(USART1) & US_CSR_RXRDY)) return 0;  
	return 0; //return USART_ReadBuffer(USART1, data, max_size); 
}
*/
uint16_t uart0_getc(void){
	return USART_ReceiveData(USART1); 
}

uint16_t uart0_putc(uint8_t ch){
	while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET); 
	USART_SendData(USART1, ch); 
	return 1; 
}


void uart1_init_default(uint32_t baud){
	
}

uint16_t uart1_getc(void){
	return USART_ReceiveData(USART2); 
}

uint16_t uart1_putc(uint8_t ch){
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); 
	USART_SendData(USART2, ch); 
	return 1; 
}
