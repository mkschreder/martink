#pragma once

struct serial_device *atmega_uart_get_adapter(void); 

void uart0_putc_direct(uint8_t ch); 
