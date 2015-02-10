#include <arch/soc.h>
#include "stm32.h"
#include "time.h"

static volatile int32_t _millis = 0, _micros = 0; 

void SysTick_Handler(void); 
void SysTick_Handler(void){
	_micros++; 
	if(_micros % 1000 == 0) {
		_millis++; 
		_micros = 0; 
	}
}
    
void tsc_init(void)
{
	SysTick_Config (SystemCoreClock / 1000000UL); 
	
	//set systick interrupt priority
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);    //4 bits for preemp priority 0 bit for sub priority
	NVIC_SetPriority(SysTick_IRQn, 0);//i want to make sure systick has highest priority amount all other interrupts
}

timestamp_t tsc_read(void)
{
	timestamp_t ts = 0; 
	//__disable_irq(); 
	ts = _millis * 1000 + _micros;
	//__enable_irq(); 
	return ts; 
}
/*
uint32_t millis(void)
{
	return _millis;
}

void delay_ms(uint32_t ms)
{
	int32_t curTime = _millis;
	while((ms-(_millis-curTime)) > 0);
}

void delay_us(uint32_t us)
{
	int32_t curTime = _micros;
	while((us-(Micros-curTime)) > 0);
}
*/
timestamp_t tsc_us_to_ticks(timestamp_t us){
	return us; 
}

timestamp_t tsc_ticks_to_us(timestamp_t ticks){
	return ticks; 
}

