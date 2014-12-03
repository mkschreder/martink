#pragma once 

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include <util/crc16.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "m328p/adc.h"
#include "m328p/i2cmaster.h"
#include "m328p/twi_slave.h"
#include "m328p/random.h"
#include "m328p/spi.h"
#include "m328p/stack.h"
#include "m328p/time.h"
#include "m328p/twi_slave.h"
#include "m328p/uart.h"
