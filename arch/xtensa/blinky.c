#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

#include <kernel/kernel.h>
#include <kernel/thread/process.h>

static const int pin = 1;
static volatile os_timer_t some_timer;

static int32_t _ms = 0; 
static void timer_ms(void* arg){
	_ms ++; 
}

static void some_timerfunc(void *arg)
{
	//Do blinky stuff
	if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & (1 << pin))
	{
		// set gpio low
		gpio_output_set(0, (1 << pin), 0, 0);
	}
	else
	{
		// set gpio high
		gpio_output_set((1 << pin), 0, 0, 0);
	}
}

PROCESS(main_process, "Main process"); 
PROCESS_THREAD(main_process, ev, data){
	PROCESS_BEGIN(); 
	while(1){
		some_timerfunc(NULL); 
		PROCESS_CURRENT()->sleep_until = _ms + 500; 
		PROCESS_WAIT_UNTIL(_ms - PROCESS_CURRENT()->sleep_until > 0); 
	}
	PROCESS_END(); 
}

void ICACHE_FLASH_ATTR user_init(void); 
void ICACHE_FLASH_ATTR user_init(void){
	// init gpio sussytem
	gpio_init();

	// configure UART TXD to be GPIO1, set as output
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1); 
	gpio_output_set(0, 0, (1 << pin), 0);

	// setup timer (500ms, repeating)
	os_timer_setfn(&some_timer, (os_timer_func_t *)timer_ms, NULL);
	os_timer_arm(&some_timer, 1, 1);

	process_init(); 
	process_start(&main_process, 0); 
	
	while(1){
		process_post(&main_process, PROCESS_EVENT_CONTINUE, 0); 
		process_run(); 
	}
}
