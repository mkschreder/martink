/* Kernel includes. */
#include <arch/soc.h>

#include <kernel/mt.h>
#include <kernel/time.h>

#include <serial/serial.h>

/* TODO Add any manufacture supplied header files can be included
here. */
//#include "hardware.h"

/* Priorities at which the tasks are created.  The event semaphore task is
given the maximum priority of ( configMAX_PRIORITIES - 1 ) to ensure it runs as
soon as the semaphore is given. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2 )
#define mainQUEUE_SEND_TASK_PRIORITY        ( tskIDLE_PRIORITY + 2 )
#define mainEVENT_SEMAPHORE_TASK_PRIORITY   ( tskIDLE_PRIORITY + 3 )
//( configMAX_PRIORITIES - 1 )

/* The rate at which data is sent to the queue, specified in milliseconds, and
converted to ticks using the pdMS_TO_TICKS() macro. */
#define mainQUEUE_SEND_PERIOD_MS            pdMS_TO_TICKS( 200 )

/* The period of the example software timer, specified in milliseconds, and
converted to ticks using the pdMS_TO_TICKS() macro. */
#define mainSOFTWARE_TIMER_PERIOD_MS        pdMS_TO_TICKS( 1000 )

/* The number of items the queue can hold.  This is 1 as the receive task
has a higher priority than the send task, so will remove items as they are added,
meaning the send task should always find the queue empty. */
#define mainQUEUE_LENGTH                    ( 1 )

/*-----------------------------------------------------------*/

/*
 * TODO: Implement this function for any hardware specific clock configuration
 * that was not already performed before main() was called.
 */

/*
 * The queue send and receive tasks as described in the comments at the top of
 * this file.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask(void *pvParameters); 
/*
 * The callback function assigned to the example software timer as described at
 * the top of this file.
 */
//static void vExampleTimerCallback( TimerHandle_t xTimer );

/*
 * The event semaphore task as described at the top of this file.
 */
//static void prvEventSemaphoreTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* The queue used by the queue send and queue receive tasks. */
//static QueueHandle_t xQueue = NULL;

/* The semaphore (in this case binary) that is used by the FreeRTOS tick hook
 * function and the event semaphore task.
 */
//static SemaphoreHandle_t xEventSemaphore = NULL;

/* The counters used by the various examples.  The usage is described in the
 * comments at the top of this file.
 */
static volatile uint32_t ulCountOfTimerCallbackExecutions = 0;
static volatile uint32_t ulCountOfItemsReceivedOnQueue = 0;
static volatile uint32_t ulCountOfReceivedSemaphores = 0;

/*-----------------------------------------------------------*/

TaskHandle_t rx_task, tx_task; 

static void init_blink(void){
	DDRB |= (1 << 5); 
	for(int c = 0; c < 5; c++){
		PORTB |= (1 << 5); 
		_delay_ms(50); 
		PORTB &= ~(1 << 5); 
		_delay_ms(50); 
	}
}

int main(void){
	_delay_ms(1000); 	
	init_blink(); 
TimerHandle_t xExampleSoftwareTimer = NULL;
#if 0
    /* Create the queue used by the queue send and queue receive tasks. */
    xQueue = xQueueCreate(     /* The number of items the queue can hold. */
                            mainQUEUE_LENGTH,
                            /* The size of each item the queue holds. */
                            sizeof( uint32_t ) );

#endif
    /* Create the semaphore used by the FreeRTOS tick hook function and the
    event semaphore task.  NOTE: A semaphore is used for example purposes,
    using a direct to task notification will be faster! */
    //xEventSemaphore = xSemaphoreCreateBinary();

    /* Create the queue receive task as described in the comments at the top
    of this file. */
    xTaskCreate(     /* The function that implements the task. */
                    prvQueueReceiveTask,
                    /* Text name for the task, just to help debugging. */
                    "Rx",
                    /* The size (in words) of the stack that should be created
                    for the task. */
                    300,
                    /* A parameter that can be passed into the task.  Not used
                    in this simple demo. */
                    NULL,
                    /* The priority to assign to the task.  tskIDLE_PRIORITY
                    (which is 0) is the lowest priority.  configMAX_PRIORITIES - 1
                    is the highest priority. */
                    mainQUEUE_RECEIVE_TASK_PRIORITY,
                    /* Used to obtain a handle to the created task.  Not used in
                    this simple demo, so set to NULL. */
                    &rx_task);

    /* Create the queue send task in exactly the same way.  Again, this is
    described in the comments at the top of the file. */
    xTaskCreate(     prvQueueSendTask,
                    "Tx",
                    300,
                    NULL,
                    mainQUEUE_SEND_TASK_PRIORITY,
                    &tx_task );
#if 0
    /* Create the software timer as described in the comments at the top of
    this file. */
    xExampleSoftwareTimer = xTimerCreate(     /* A text name, purely to help
                                            debugging. */
                                            "LEDTimer",
                                            /* The timer period, in this case
                                            1000ms (1s). */
                                            mainSOFTWARE_TIMER_PERIOD_MS,
                                            /* This is a periodic timer, so
                                            xAutoReload is set to pdTRUE. */
                                            pdTRUE,
                                            /* The ID is not used, so can be set
                                            to anything. */
                                            ( void * ) 0,
                                            /* The callback function that switches
                                            the LED off. */
                                            vExampleTimerCallback
                                        );

    /* Start the created timer.  A block time of zero is used as the timer
    command queue cannot possibly be full here (this is the first timer to
    be created, and it is not yet running). */
    xTimerStart( xExampleSoftwareTimer, 0 );
#endif
    /* Start the tasks and timer running. */

    vTaskStartScheduler();

    /* If all is well, the scheduler will now be running, and the following line
    will never be reached.  If the following line does execute, then there was
    insufficient FreeRTOS heap memory available for the idle and/or timer tasks
    to be created.  See the memory management section on the FreeRTOS web site
    for more details.  */
    for( ;; );
}
/*-----------------------------------------------------------*/
#if 0
static void some_driver_op(void){
	//printf("driver op called from %p\n", xTaskGetCurrentTaskHandle()); 
	printf("delay..\n"); 
	vTaskDelay(pdMS_TO_TICKS(500)); 
	//printf("driver yield called from %p\n", xTaskGetCurrentTaskHandle()); 
}
#endif
#include <stdio.h>
#if 0
static void vExampleTimerCallback( TimerHandle_t xTimer )
{
    /* The timer has expired.  Count the number of times this happens.  The
    tIMEr that calls this function is an auto re-load timer, so it will
    execute periodically. */
	//printf("timer tick\n"); 
    ulCountOfTimerCallbackExecutions++;
}
#endif
#include <stdlib.h>
#include <adc/adc.h>

/*-----------------------------------------------------------*/
static void prvQueueSendTask( void *pvParameters ){
	for( ;; ){
		char ch[2] = {0}; 
		printk("enter number for tx: "); 	
		serial_read(_default_system_console, ch, 1); 	
		printk("tx got %d\r\n", atoi(ch)); 	
		uint16_t adc = adc_read_channel(adc_get_device(0), 0); 
		printk("adc value: %d\r\n", adc); 
		PORTB |= (1 << 5); 
		vTaskDelay(pdMS_TO_TICKS(500)); 
		PORTB &= ~(1 << 5); 
		_delay_ms(500); 
		vTaskDelay(pdMS_TO_TICKS(500)); 
	}
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask( void *pvParameters ){
	for( ;; ){
		printk("rx ticks: %lu\r\n", (uint32_t)(tsc_ticks_to_us(timestamp_now()) / 1000)); 
		printk("ticks_to_us(10): %lu\r\n", (uint32_t)tsc_ticks_to_us(tsc_us_to_ticks(10))); 
		printk("us_to_ticks(10): %lu\r\n", (uint32_t)tsc_us_to_ticks(10)); 
		for(int c = 0; c < 5; c++){
			PORTB |= (1 << 5); 
			vTaskDelay(pdMS_TO_TICKS(50)); 
			PORTB &= ~(1 << 5); 
			vTaskDelay(pdMS_TO_TICKS(50)); 
		}
		vTaskDelay(pdMS_TO_TICKS(2000)); 
	}
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void ); 
void vApplicationTickHook( void )
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	static uint32_t ulCount = 0;

    /* The RTOS tick hook function is enabled by setting configUSE_TICK_HOOK to
    1 in FreeRTOSConfig.h.

    "Give" the semaphore on every 500th tick interrupt. */
    ulCount++;
    if( ulCount >= 500UL )
    {
        /* This function is called from an interrupt context (the RTOS tick
        interrupt),    so only ISR safe API functions can be used (those that end
        in "FromISR()".

        xHigherPriorityTaskWoken was initialised to pdFALSE, and will be set to
        pdTRUE by xSemaphoreGiveFromISR() if giving the semaphore unblocked a
        task that has equal or higher priority than the interrupted task.
        NOTE: A semaphore is used for example purposes.  In a real application it
        might be preferable to use a direct to task notification,
        which will be faster and use less RAM. */
        //xSemaphoreGiveFromISR( xEventSemaphore, &xHigherPriorityTaskWoken );
        ulCount = 0UL;
    }

    /* If xHigherPriorityTaskWoken is pdTRUE then a context switch should
    normally be performed before leaving the interrupt (because during the
    execution of the interrupt a task of equal or higher priority than the
    running task was unblocked).  The syntax required to context switch from
    an interrupt is port dependent, so check the documentation of the port you
    are using.

    In this case, the function is running in the context of the tick interrupt,
    which will automatically check for the higher priority task to run anyway,
    so no further action is required. */
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook( void ); 
void vApplicationMallocFailedHook( void )
{
    /* The malloc failed hook is enabled by setting
    configUSE_MALLOC_FAILED_HOOK to 1 in FreeRTOSConfig.h.

    Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    for( ;; ){
		PORTB |= (1<<5); 
		_delay_ms(100); 
		PORTB &= ~(1<<5); 
		_delay_ms(100); 
	}
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ); 
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName ){
    ( void ) pcTaskName;
    ( void ) xTask;

    /* Run time stack overflow checking is performed if
    configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  pxCurrentTCB can be
    inspected in the debugger if the task name passed into this function is
    corrupt. */
    for( ;; ){
		PORTB |= (1<<5); 
		_delay_ms(100); 
		PORTB &= ~(1<<5); 
		_delay_ms(100); 
	}

}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void ); 
void vApplicationIdleHook( void ){
	static int idle_count = 0; 
	if(idle_count % 100000000 == 0) {
		//printk("idle %d\r\n", idle_count); 
	//	kprintf("idle count %d\n", idle_count); 
	}
	idle_count++; 
}

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint16_t *pusIdleTaskStackSize ); 
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint16_t *pusIdleTaskStackSize )
{
    /* Setting this parameter to NULL will result in the Idle task's TCB being
    allocated dynamically. */
    *ppxIdleTaskTCBBuffer = NULL;

    /* Setting this parameter to NULL will result in the Idle task's stack being
    allocated dynamically. */
    *ppxIdleTaskStackBuffer = NULL;

    /* The size of the stack to allocate - in words, NOT in bytes! */
    *pusIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

