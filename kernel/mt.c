#include "mt.h"

#ifdef CONFIG_FREERTOS
static const thread_attr_t _default_attrs = {
	.name = NULL, 
	.stack_size = 300, // TODO: adjust
	.priority = tskIDLE_PRIORITY + 1
}; 

void thread_attr_init(struct thread_attrs *attr){
	memcpy(attr, &_default_attrs, sizeof(thread_attr_t)); 
}

int thread_create(thread_t *thread, const thread_attr_t *attrs, void (*start_routine)(void*), void *arg){
	if(!attrs) attrs = &_default_attrs; 
    xTaskCreate(     
		/* The function that implements the task. */
		start_routine,
		/* Text name for the task, just to help debugging. */
		attrs->name,
		/* The size (in words) of the stack that should be created
		for the task. */
		attrs->stack_size,
		/* A parameter that can be passed into the task.  Not used
		in this simple demo. */
		arg,
		/* The priority to assign to the task.  tskIDLE_PRIORITY
		(which is 0) is the lowest priority.  configMAX_PRIORITIES - 1
		is the highest priority. */
		attrs->priority,
		/* Used to obtain a handle to the created task.  Not used in
		this simple demo, so set to NULL. */
		thread);
	return 0; 
}

// FreeRTOS hooks..

#include <arch/soc.h>

void vApplicationTickHook( void ); 
void vApplicationTickHook( void ){

}

void vApplicationMallocFailedHook( void ); 
void vApplicationMallocFailedHook( void ){
	// panic 
	for( ;; ){
		platform_panic_noblock(); 
	}
}

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ); 
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ){
    ( void ) pcTaskName;
    ( void ) xTask;

    for( ;; ){
		platform_panic_noblock(); 
	}
}
/*-----------------------------------------------------------*/

#include <serial/serial.h>
#include <avr/sleep.h>
#include <avr/power.h>

void vApplicationIdleHook( void ); 
void vApplicationIdleHook( void ){
	cpu_relax(); 
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint16_t *pusIdleTaskStackSize ); 
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint16_t *pusIdleTaskStackSize ){
    /* Setting this parameter to NULL will result in the Idle task's TCB being
    allocated dynamically. */
    *ppxIdleTaskTCBBuffer = NULL;

    /* Setting this parameter to NULL will result in the Idle task's stack being
    allocated dynamically. */
    *ppxIdleTaskStackBuffer = NULL;

    /* The size of the stack to allocate - in words, NOT in bytes! */
    *pusIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
#endif
