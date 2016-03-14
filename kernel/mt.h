#pragma once

#ifdef CONFIG_FREERTOS
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/croutine.h"
#include "freertos/timers.h"
#include "freertos/semphr.h"
#include "freertos/port.h"
#else
#error "You need to enable FreeRTOS support in the config before including this header!"
#endif

/* Following are convenience definitions for efficient use of most common constructs */

typedef StaticSemaphore_t mutex_t; 
typedef StaticSemaphore_t sem_t; 

#define mutex_init(sem) xSemaphoreCreateMutexStatic((sem))
#define mutex_lock_timeout(sem, timeout) xSemaphoreTake((sem), (timeout))
#define mutex_lock(sem) xSemaphoreTake((sem), portMAX_DELAY)
#define mutex_unlock(sem) xSemaphoreGive((sem))
#define mutex_unlock_from_isr(sem) xSemaphoreGiveFromISR((sem), NULL)

#define sem_init(sem, max, starting) xSemaphoreCreateCountingStatic((max), (starting), (sem))
#define sem_give(sem) xSemaphoreGive((sem))
#define sem_give_from_isr(sem) xSemaphoreGiveFromISR((sem), NULL)
#define sem_take_timeout(sem, timeout) xSemawhoreTake((sem), (timeout))
#define sem_take(sem) xSemaphoreTake((sem), portMAX_DELAY)
#define sem_count(sem) uxSemaphoreGetCount((sem))

/* Threads and delays */
#define msleep(ms) vTaskDelay(pdMS_TO_TICKS((ms))); 
