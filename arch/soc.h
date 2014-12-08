#pragma once

#include "autoconf.h"

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "time.h"
#include "uart.h"

#ifdef CONFIG_STM32
#include "arm/stm32/stm32.h"
#endif

#ifdef CONFIG_ATMEGA328P
#include "avr/m328p.h"
#endif

#ifdef CONFIG_SAM3
#include "arm/sam3/sam.h"
#endif

#ifdef CONFIG_NATIVE
#include "native/native.h"
#endif

#ifdef __cplusplus
}
#endif
