#pragma once 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/crc16.h>
#include <util/delay.h>
#include <util/atomic.h>

#ifdef CONFIG_ADC
#include "m328p/adc.h"
#endif
#ifdef CONFIG_I2C
#include "m328p/i2cmaster.h"
#include "m328p/twi_slave.h"
#include "m328p/twi_slave.h"
#endif

#ifdef CONFIG_SPI
#include "m328p/spi.h"
#endif

#ifdef CONFIG_UART
#include "m328p/uart.h"
#endif

#include "m328p/random.h"
#include "m328p/stack.h"
#include "m328p/time.h"
