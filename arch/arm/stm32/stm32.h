#pragma once

#if defined(CONFIG_CPU_STM32F10X)
#include "CMSIS/stm32f10x.h"
#include "f10x/stm32f10x_usart.h"
#include "f10x/stm32f10x_i2c.h"
#include "f10x/stm32f10x_gpio.h"
#include "f10x/stm32f10x_spi.h"
#include "f10x/stm32f10x_rcc.h"
#include "f10x/stm32f10x_tim.h"
#include "f10x/stm32f10x_adc.h"
#include "f10x/stm32f10x_dma.h"
#include "f10x/stm32f10x_exti.h"
#include "f10x/misc.h"
#endif

#include "uart.h"
#include "gpio.h"
#include "time.h"
#include "spi.h"
#include "twi.h"
#include "pwm.h"
#include "adc.h"
#include "clock.h"

// TODO: define these
#define cpu_relax() {}
#define cpu_powerdown() {}
#define platform_panic_noblock() {} 
// ??
#define STM32_DELAY_US_MULT 1

static inline void udelay(uint32_t us){
	us *= STM32_DELAY_US_MULT;
	us--;
	__asm volatile (" mov r0, %0 \n"
	"loop: subs r0, #1 \n"
	" bhi loop \n":"=r" (us));
}

void EnterFlashUpgrade(void); 

//#define eeprom_update_block(a, b, c) {}
//#define eeprom_read_block(a, b, c) {}
