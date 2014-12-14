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

#include "uart.h"


#define DECLARE_UART0_RX_INTERRUPT(cbuf_rx_buf, u8_err_var) \
void _uart0_rx_interrupt__(void){}

#define DECLARE_UART0_TX_INTERRUPT() void _uart0_tx_interrupt__(void){}

#define DECLARE_UART0_DRE_INTERRUPT(cbuf_tx_buf) \
void _uart0_dre_interrupt__(void){}

/*
#ifdef CONFIG_HAVE_UART0
void PFDECL(CONFIG_UART0_NAME, init, uint32_t baud);
uint16_t PFDECL(CONFIG_UART0_NAME, printf, const char *fmt, ...);
size_t PFDECL(CONFIG_UART0_NAME, write, const uint8_t *data, size_t max_size);
size_t PFDECL(CONFIG_UART0_NAME, read, uint8_t *data, size_t max_size);
uint16_t PFDECL(CONFIG_UART0_NAME, getc, void);
void PFDECL(CONFIG_UART0_NAME, putc, uint8_t ch); 
#endif

#ifdef CONFIG_HAVE_UART1
void PFDECL(CONFIG_UART1_NAME, init, uint32_t baud);
uint16_t PFDECL(CONFIG_UART1_NAME, printf, const char *fmt, ...);
size_t PFDECL(CONFIG_UART1_NAME, write, const uint8_t *data, size_t max_size);
size_t PFDECL(CONFIG_UART1_NAME, read, uint8_t *data, size_t max_size);
uint16_t PFDECL(CONFIG_UART1_NAME, getc, void);
void PFDECL(CONFIG_UART1_NAME, putc, uint8_t ch); 
#endif
*/
