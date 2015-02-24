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

#include <util/cbuf.h>

#include "uart.h"

#define UART_RX_BUFFER_SIZE 256
#define UART_TX_BUFFER_SIZE 256

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

static struct cbuf rx_buffers[sizeof(_devices)/sizeof(struct uart_device)]; 
static uint8_t rx_data[sizeof(_devices)/sizeof(struct uart_device)][UART_RX_BUFFER_SIZE]; 
static struct cbuf tx_buffers[sizeof(_devices)/sizeof(struct uart_device)]; 
static uint8_t tx_data[sizeof(_devices)/sizeof(struct uart_device)][UART_TX_BUFFER_SIZE]; 
/*
DECLARE_STATIC_CBUF(uart0_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart1_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart2_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart3_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart4_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
DECLARE_STATIC_CBUF(uart5_rx_buf, uint8_t, UART_RX_BUFFER_SIZE);
*/

int8_t uart_init(uint8_t dev_id, uint32_t baud){
	USART_InitTypeDef usartConfig;
	
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	
	if(dev_id >= count) return -1; 
	
	const struct uart_device *conf = &_devices[dev_id]; 
	USART_TypeDef *dev = _devices[dev_id].dev; 
	
	cbuf_init(&rx_buffers[dev_id], rx_data[dev_id], UART_RX_BUFFER_SIZE); 
	cbuf_init(&tx_buffers[dev_id], tx_data[dev_id], UART_TX_BUFFER_SIZE); 
	
	USART_DeInit(dev); 
	
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
	USART_ITConfig(dev, USART_IT_TXE, ENABLE);
	
	return 0; 
}

void uart_deinit(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return; 
	USART_TypeDef *dev = _devices[dev_id].dev; 
	
	USART_DeInit(dev); 
}

int8_t		uart_set_baudrate(uint8_t dev_id, uint32_t baud){
	USART_InitTypeDef usartConfig;
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	USART_TypeDef *dev = _devices[dev_id].dev; 
	
	USART_Cmd(dev, DISABLE);

	usartConfig.USART_BaudRate = baud;
	usartConfig.USART_WordLength = USART_WordLength_8b;
	usartConfig.USART_StopBits = USART_StopBits_1;
	usartConfig.USART_Parity = USART_Parity_No;
	usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usartConfig.USART_HardwareFlowControl =
			 USART_HardwareFlowControl_None;
	USART_Init(dev, &usartConfig);
	
	USART_Cmd(dev, ENABLE);
	
	return 0; 
}

uint16_t uart_getc(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return SERIAL_NO_DATA; 
	USART_TypeDef *dev = _devices[dev_id].dev; 

	uint16_t ret = SERIAL_NO_DATA; 
	USART_ITConfig(dev, USART_IT_RXNE, DISABLE);
	
	// no need for critical section because cbuffer is threadsafe
	if(cbuf_get_waiting(&rx_buffers[dev_id]) > 0) { ret = cbuf_get(&rx_buffers[dev_id]); }
	
	USART_ITConfig(dev, USART_IT_RXNE, ENABLE);
	return ret; 
}

int8_t uart_putc(uint8_t dev_id, uint8_t ch){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	
	USART_TypeDef *dev = _devices[dev_id].dev; 
	
	// ok so the policy is currently to block and wait
	while(cbuf_is_full(&tx_buffers[dev_id])); 
	
	USART_ITConfig(dev, USART_IT_TXE, DISABLE);
	cbuf_put(&tx_buffers[dev_id], ch);
	USART_ITConfig(dev, USART_IT_TXE, ENABLE);
	
	//while(!(dev->SR & USART_SR_TXE));
	//dev->DR = ch;  
	return 0; 
}

static void USART_Handler(USART_TypeDef *dev, struct cbuf *rx_buf, struct cbuf *tx_buf){
	if(USART_GetITStatus(dev, USART_IT_RXNE) != RESET){
		USART_ClearITPendingBit(dev, USART_IT_RXNE);
		unsigned char ch = USART_ReceiveData(dev) & 0xff;
		cbuf_put_isr(rx_buf, ch); 
	}
	if(USART_GetITStatus(dev, USART_IT_TXE) != RESET){
		USART_ClearITPendingBit(dev, USART_IT_TXE);
		if(!cbuf_is_empty(tx_buf))
			USART_SendData(dev, cbuf_get(tx_buf)); 
		else
			USART_ITConfig(dev, USART_IT_TXE, DISABLE);
	}
}

void USART1_IRQHandler(void);
void USART1_IRQHandler(void)
{
	USART_Handler(USART1, &rx_buffers[0], &tx_buffers[0]); 
	/*
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
		unsigned char ch = USART_ReceiveData(USART1) & 0xff;
		cbuf_put_isr(&rx_buffers[0], ch); 
	}
	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET){
		USART_ClearITPendingBit(USART1, USART_IT_TXE);
		if(!cbuf_is_empty(&tx_buffers[0]))
			UART1->DR = cbuf_get(&tx_buffers[0]); 
		else
			USART_ITConfig(dev, USART_IT_TXE, DISABLE);
	}*/
}


void USART2_IRQHandler(void);
void USART2_IRQHandler(void)
{
	USART_Handler(USART2, &rx_buffers[1], &tx_buffers[1]); 
	/*
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);
		unsigned char ch = USART_ReceiveData(USART2);
		cbuf_put_isr(&rx_buffers[1], ch); 
	}*/
}

void USART3_IRQHandler(void);
void USART3_IRQHandler(void)
{	
	USART_Handler(USART3, &rx_buffers[2], &tx_buffers[2]); 
	/*if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);
		unsigned char ch = USART_ReceiveData(USART3);
		cbuf_put_isr(&rx_buffers[2], ch); 
	}*/
}

uint16_t uart_waiting(uint8_t dev_id){
	uint8_t count = sizeof(_devices) / sizeof(_devices[0]); 
	if(dev_id >= count) return -1; 
	
	return cbuf_get_waiting(&rx_buffers[dev_id]); 
}
