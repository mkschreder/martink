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

#include <static_cbuf.h>

#include "uart.h"

#define UART_RX_BUFFER_SIZE 256

DECLARE_STATIC_CBUF(uart0_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);

void uart0_init_default(uint32_t baud){
	USART_InitTypeDef usartConfig;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |
												 RCC_APB2Periph_GPIOA |
												 RCC_APB2Periph_AFIO, ENABLE);
	USART_Cmd(USART1, ENABLE);

	usartConfig.USART_BaudRate = baud;
	usartConfig.USART_WordLength = USART_WordLength_8b;
	usartConfig.USART_StopBits = USART_StopBits_1;
	usartConfig.USART_Parity = USART_Parity_No;
	usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usartConfig.USART_HardwareFlowControl =
			 USART_HardwareFlowControl_None;
	USART_Init(USART1, &usartConfig);
	
	GPIO_InitTypeDef gpioConfig;

	//PA9 = USART1.TX => Alternative Function Output
	gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioConfig.GPIO_Pin = GPIO_Pin_9;
	gpioConfig.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &gpioConfig);

	//PA10 = USART1.RX => Input
	gpioConfig.GPIO_Mode = GPIO_Mode_IPU;
	gpioConfig.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &gpioConfig);
	
	NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

uint16_t uart0_getc(void){
	uint16_t ret = SERIAL_NO_DATA; 
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	if(!cbuf_is_empty(&uart0_rx_buf)) {
		ret = cbuf_get(&uart0_rx_buf);
	}
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	return ret; 
	//if(!(USART1->SR & USART_SR_RXNE)) return SERIAL_NO_DATA; 
	//return USART1->DR & 0xff; 
}

uint16_t uart0_putc(uint8_t ch){
	while(!(USART1->SR & USART_SR_TXE));
	USART1->DR = ch;  
	return 1; 
}

void USART1_IRQHandler(void);
void USART1_IRQHandler(void)
{
	if(USART1->SR & USART_FLAG_RXNE){
		char ch = USART_ReceiveData(USART1);
		if(!cbuf_is_full(&uart0_rx_buf)){ 
			cbuf_put(&uart0_rx_buf, ch); 
		} 
	}
}

uint16_t uart0_waiting(void){
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);
	size_t cn = cbuf_get_data_count(&uart0_rx_buf);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	return cn; 
	//if(USART1->SR & USART_SR_RXNE) return 1; 
	//return 0; 
}

void uart1_init_default(uint32_t baud){
	(void)(baud);
}

uint16_t uart1_getc(void){
	return USART_ReceiveData(USART2); 
}

uint16_t uart1_putc(uint8_t ch){
	while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET); 
	USART_SendData(USART2, ch); 
	return 1; 
}
