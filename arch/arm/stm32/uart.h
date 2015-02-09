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

#pragma once

void uart0_init_default(uint32_t baud);
uint16_t uart0_getc(void); 
uint16_t uart0_putc(uint8_t ch);
uint16_t uart0_waiting(void);

void uart1_init_default(uint32_t baud); 
uint16_t uart1_getc(void); 
uint16_t uart1_putc(uint8_t ch);
static inline int8_t uart1_waiting(void){
	return 0; 
}

/*
void uart0_init_default(uint16_t baudrate);

#define uart0_waiting() (USART_IsDataAvailable(USART0))
#define uart0_flush() do {} while(0)
#define uart0_getc() ((uart0_waiting())?USART_GetChar(USART0):UART_NO_DATA)
#define uart0_putc(data) (USART_PutChar(USART0, data), 0)

void uart1_init_default(uint16_t baud); 
#define uart1_waiting() (USART_IsDataAvailable(USART1))
#define uart1_flush() do {} while(0)
#define uart1_getc() ((uart0_waiting())?USART_GetChar(USART1):UART_NO_DATA)
#define uart1_putc(data) (USART_PutChar(USART1, data), 0)
*/
