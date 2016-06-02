#pragma once

#include "config.h"

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
#define schedule_start() vTaskStartScheduler()

#define thread_self() xTaskGetCurrentTaskHandle()

typedef TaskHandle_t thread_t; 

typedef struct thread_attrs {
	const char *name; 
	uint8_t priority; 
	int stack_size; 
} thread_attr_t; 

void thread_attr_init(struct thread_attrs *attr); 
int thread_create(thread_t *thread, const thread_attr_t *attrs, void (*start_proc)(void*), void *arg); 

// TODO: move this somewhere else

#include <string.h>
static inline void* kzmalloc(size_t size) {
	void *ret = pvPortMalloc(size); 
	memset(ret, 0, size); 
	return ret; 
}
