#pragma once

#ifdef CONFIG_FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/croutine.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#else
#error "You need to enable FreeRTOS support in the config before including this header!"
#endif
