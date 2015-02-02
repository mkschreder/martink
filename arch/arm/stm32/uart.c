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

#define gpioa_clock_en() 

static void _uart_init(USART_TypeDef *uart, uint32_t baud){
	// turn on uart
	uart->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_UE; 

	// compute baud rate
	uart->BRR = F_CPU/baud;
	
}

void uart0_init_default(uint32_t baud){
	_uart_init(USART1, baud); 
	
	RCC->APB2ENR |= 0
		// Turn on USART1
		| RCC_APB2ENR_USART1EN
		// Turn on IO Port A
		| RCC_APB2ENR_IOPAEN
		// Turn on the alternate function block
		| RCC_APB2ENR_AFIOEN;
		
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
}

uint16_t uart0_getc(void){
	if(!(USART1->SR & USART_SR_RXNE)) return SERIAL_NO_DATA; 
	return USART1->DR & 0xff; 
}

uint16_t uart0_putc(uint8_t ch){
	while(!(USART1->SR & USART_SR_TXE));
	USART1->DR = ch;  
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
