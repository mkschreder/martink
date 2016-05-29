#include <arch/soc.h>
#include <kernel/time.h>
#include "stm32.h"
#include "time.h"

static volatile uint32_t _ticks = 0; 
static long _clk_per_us = 1; 

#define SYSTICK_DIV 1000UL

//#define SYSTICK_INTERVAL 1000UL //(1000000UL / SYSTICK_DIV)

void SysTick_Handler(void); 
void SysTick_Handler(void){
	++_ticks; 
}
    
static void __init tsc_init(void){
	RCC_ClocksTypeDef RCC_Clocks;

	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.SYSCLK_Frequency / 1000UL); 
	_clk_per_us = RCC_Clocks.SYSCLK_Frequency / 1000000UL; 
  
	//SysTick_Config (SystemCoreClock / 1000000UL); 
	
	//set systick interrupt priority
	//4 bits for preemp priority 0 bit for sub priority
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);    
	NVIC_SetPriority(SysTick_IRQn, 0);
}

long long tsc_read(void){
	timestamp_t ret; 
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ret = _ticks * 1000 + 1000 - (SysTick->VAL / _clk_per_us); 
	}
	return ret; 
}

long long tsc_us_to_ticks(long long us){
	return us; 
}

long long tsc_ticks_to_us(long long ticks){
	return ticks; 
}

