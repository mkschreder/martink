#pragma once

#if defined(CONFIG_BOARD_MULTIWII)
#include "multiwii.h"
#elif defined(CONFIG_BOARD_PROMINI_RXTX)
#include "promini_rftrx.h"
#elif defined(CONFIG_BOARD_ARDUINO_DUE)
#include "arduino_due.h"
#elif defined(CONFIG_BOARD_STM32F103)
#include "stm32f103.h"
#endif
