#pragma once

#include "uart.h"

void __uart0_init__(uint32_t baud);
uint16_t __uart0_printf__(const char *fmt, ...);
