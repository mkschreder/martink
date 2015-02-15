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

#define UART_RX_BUFFER_SIZE 64

struct uart_device {
	USART_TypeDef *dev; 
	GPIO_TypeDef *gpio; 
	int rcc_gpio; 
	int rcc_id; 
	int apb_id; 
	int out_pins, in_pins; 
	int irq; 
}; 

static const struct uart_device _devices[] = {
	{
		.dev = USART1, 
		.gpio = GPIOA, 
		.rcc_gpio = RCC_APB2Periph_GPIOA, 
		.rcc_id = RCC_APB2Periph_USART1, 
		.apb_id = 2, 
		.out_pins = GPIO_Pin_9, 
		.in_pins = GPIO_Pin_10, 
		.irq = USART1_IRQn
	}, 
	{
		.dev = USART2, 
		.gpio = GPIOA, 
		.rcc_gpio = RCC_APB2Periph_GPIOA, 
		.rcc_id = RCC_APB1Periph_USART2, 
		.apb_id = 1, 
		.out_pins = GPIO_Pin_2, 
		.in_pins = GPIO_Pin_3, 
		.irq = USART2_IRQn
	}, 
	{
		.dev = USART3, 
		.gpio = GPIOB, 
		.rcc_gpio = RCC_APB2Periph_GPIOB, 
		.rcc_id = RCC_APB1Periph_USART3, 
		.apb_id = 1, 
		.out_pins = GPIO_Pin_10, 
		.in_pins = GPIO_Pin_11, 
		.irq = USART3_IRQn
	}
}; 

DECLARE_STATIC_CBUF(uart0_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart1_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart2_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart3_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart4_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart5_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);

int8_t uart_init(uint8_t dev_id, uint32_t baud){
	USART_InitTypeDef usartConfig;
	
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	
	if(dev_id >= count) return -1; 
	
	const struct uart_device *conf = &_devices[dev_id]; 
	USART_TypeDef *dev = _devices[dev_id].dev; 
	
	if(conf->apb_id == 2){
		RCC_APB2PeriphClockCmd(conf->rcc_id, ENABLE);
	} else if(conf->apb_id == 1){
		RCC_APB1PeriphClockCmd(conf->rcc_id, ENABLE);
	} else {
		return -1; 
	}
	
	RCC_APB2PeriphClockCmd(conf->rcc_gpio | RCC_APB2Periph_AFIO, ENABLE);
	
	USART_Cmd(dev, ENABLE);

	usartConfig.USART_BaudRate = baud;
	usartConfig.USART_WordLength = USART_WordLength_8b;
	usartConfig.USART_StopBits = USART_StopBits_1;
	usartConfig.USART_Parity = USART_Parity_No;
	usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usartConfig.USART_HardwareFlowControl =
			 USART_HardwareFlowControl_None;
	USART_Init(dev, &usartConfig);
	
	GPIO_InitTypeDef gpioConfig;

	// configure tx pin
	gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioConfig.GPIO_Pin = conf->out_pins;
	gpioConfig.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(conf->gpio, &gpioConfig);

	// configure rx pin
	gpioConfig.GPIO_Mode = GPIO_Mode_IPU;
	gpioConfig.GPIO_Pin = conf->in_pins;
	GPIO_Init(conf->gpio, &gpioConfig);
	
	NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the USARTx Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = conf->irq;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
	USART_ITConfig(dev, USART_IT_RXNE, ENABLE);
	
	return 0; 
}

uint16_t uart_getc(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return SERIAL_NO_DATA; 
	
	USART_TypeDef *dev = _devices[dev_id].dev; 

	uint16_t ret = SERIAL_NO_DATA; 
	USART_ITConfig(dev, USART_IT_RXNE, DISABLE);
	
	// TODO: can probably be done better, but for now cbuf works this way..
	switch(dev_id){
		case 0: 
			if(!cbuf_is_empty(&uart0_rx_buf)) { ret = cbuf_get(&uart0_rx_buf); }
			break;
		case 1: 
			if(!cbuf_is_empty(&uart1_rx_buf)) { ret = cbuf_get(&uart1_rx_buf); }
			break;
		case 2: 
			if(!cbuf_is_empty(&uart2_rx_buf)) { ret = cbuf_get(&uart2_rx_buf); }
			break;
		case 3: 
			if(!cbuf_is_empty(&uart3_rx_buf)) { ret = cbuf_get(&uart3_rx_buf); }
			break;
		case 4: 
			if(!cbuf_is_empty(&uart4_rx_buf)) { ret = cbuf_get(&uart4_rx_buf); }
			break;
		case 5: 
			if(!cbuf_is_empty(&uart5_rx_buf)) { ret = cbuf_get(&uart5_rx_buf); }
			break; 
	}
	USART_ITConfig(dev, USART_IT_RXNE, ENABLE);
	return ret; 
}

int8_t uart_putc(uint8_t dev_id, uint8_t ch){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	
	USART_TypeDef *dev = _devices[dev_id].dev; 
	
	while(!(dev->SR & USART_SR_TXE));
	dev->DR = ch;  
	return 0; 
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


void USART2_IRQHandler(void);
void USART2_IRQHandler(void)
{
	if(USART2->SR & USART_FLAG_RXNE){
		char ch = USART_ReceiveData(USART2);
		if(!cbuf_is_full(&uart3_rx_buf)){ 
			cbuf_put(&uart3_rx_buf, ch); 
		} 
	}
}


void USART3_IRQHandler(void);
void USART3_IRQHandler(void)
{
	if(USART3->SR & USART_FLAG_RXNE){
		char ch = USART_ReceiveData(USART3);
		if(!cbuf_is_full(&uart3_rx_buf)){ 
			cbuf_put(&uart3_rx_buf, ch); 
		} 
	}
}

uint16_t uart_waiting(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	
	USART_TypeDef *dev = _devices[dev_id].dev; 
	size_t cn = 0; 
	
	USART_ITConfig(dev, USART_IT_RXNE, DISABLE);
	switch(dev_id){
		case 0: cn = cbuf_get_data_count(&uart0_rx_buf); break; 
		case 1: cn = cbuf_get_data_count(&uart1_rx_buf); break; 
		case 2: cn = cbuf_get_data_count(&uart2_rx_buf); break; 
		case 3: cn = cbuf_get_data_count(&uart3_rx_buf); break; 
		case 4: cn = cbuf_get_data_count(&uart4_rx_buf); break; 
		case 5: cn = cbuf_get_data_count(&uart5_rx_buf); break; 
	}
	USART_ITConfig(dev, USART_IT_RXNE, ENABLE);
	return cn; 
}
