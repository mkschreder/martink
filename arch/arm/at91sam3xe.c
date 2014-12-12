/**
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/
#include <arch/soc.h>

static void __phantom_handler(void) { while(1); }
 
void NMI_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void HardFault_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void MemManage_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void BusFault_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("__phantom_handler")));
void DebugMon_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void SVC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PendSV_Handler    (void) __attribute__ ((weak, alias("__phantom_handler")));
void SysTick_Handler(void) { TimeTick_Increment(); }
void SUPC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RSTC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void WDT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void PMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UART_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SMC_INSTANCE_
void SMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_SDRAMC_INSTANCE_
void SDRAMC_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PIOA_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PIOB_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_PIOC_INSTANCE_
void PIOC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOD_INSTANCE_
void PIOD_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOE_INSTANCE_
void PIOE_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOF_INSTANCE_
void PIOF_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void USART0_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART1_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART2_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_USART3_INSTANCE_
void USART3_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void HSMCI_Handler      (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void SPI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SPI1_INSTANCE_
void SPI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void SSC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC0_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC1_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC2_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC3_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC4_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC5_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_TC2_INSTANCE_
void TC6_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC7_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC8_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PWM_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void ADC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void DACC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void DMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UOTGHS_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void TRNG_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_EMAC_INSTANCE_
void EMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void CAN0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void CAN1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));

/********************************************************************************
 * Controller initialisation. Call this function in main(), which is itself called
 * from Reset_Handler()
 ********************************************************************************/
void __libc_init_array(void);

void cpu_init(void)
{
  /*
   * SAM System init: Initializes the PLL / clock.
   * Defined in CMSIS/ATMEL/sam3xa/source/system_sam3xa.c
   */
  SystemInit();
  /*
   * Config systick interrupt timing, core clock is in microseconds --> 1ms
   * Defined in CMSIS/CMSIS/include/core_cm3.h
   */
   SysTick_Config(SystemCoreClock / 1000);
   
  //if (SysTick_Config(SystemCoreClock / 1000)) while (1);
 
  /*
   * No watchdog now
   *
   */
  WDT_Disable(WDT);
 
  /*
   * GCC libc init, also done in Reset_Handler()
   */
}

timeout_t time_get_clock(void){
	return GetTickCount(); 
}

timeout_t time_us_to_clock(timeout_t us){
	return us / 1000; 
}

timeout_t time_clock_to_us(timeout_t clock){
	return clock * 1000; 
}

struct PinDecl {
	Pio *pio; 
	int id; 
}; 

static const struct PinDecl _pins[] = {
	{PIOA, PIO_PA0}, //0
	{PIOA, PIO_PA1},
	{PIOA, PIO_PA2},
	{PIOA, PIO_PA3},
	{PIOA, PIO_PA4},
	{PIOA, PIO_PA5},
	{PIOA, PIO_PA6},
	{PIOA, PIO_PA7},
	{PIOA, PIO_PA8},
	{PIOA, PIO_PA9},
	{PIOA, PIO_PA10},
	{PIOA, PIO_PA11},
	{PIOA, PIO_PA12},
	{PIOA, PIO_PA13},
	{PIOA, PIO_PA14},
	{PIOA, PIO_PA15},
	{PIOA, PIO_PA16},
	{PIOA, PIO_PA17},
	{PIOA, PIO_PA18},
	{PIOA, PIO_PA19},
	{PIOA, PIO_PA20},
	{PIOA, PIO_PA21},
	{PIOA, PIO_PA22},
	{PIOA, PIO_PA23},
	{PIOA, PIO_PA24},
	{PIOA, PIO_PA25},
	{PIOA, PIO_PA26},
	{PIOA, PIO_PA27},
	{PIOA, PIO_PA28},
	{PIOA, PIO_PA29},
	{PIOA, PIO_PA30A_SPI0_NPCS2},
	{PIOA, PIO_PA31A_SPI0_NPCS3},
	{PIOB, PIO_PB0},
	{PIOB, PIO_PB1},
	{PIOB, PIO_PB2},
	{PIOB, PIO_PB3},
	{PIOB, PIO_PB4},
	{PIOB, PIO_PB5},
	{PIOB, PIO_PB6},
	{PIOB, PIO_PB7},
	{PIOB, PIO_PB8},
	{PIOB, PIO_PB9},
	{PIOB, PIO_PB10},
	{PIOB, PIO_PB11},
	{PIOB, PIO_PB12},
	{PIOB, PIO_PB13},
	{PIOB, PIO_PB14},
	{PIOB, PIO_PB15},
	{PIOB, PIO_PB16},
	{PIOB, PIO_PB17},
	{PIOB, PIO_PB18},
	{PIOB, PIO_PB19},
	{PIOB, PIO_PB20},
	{PIOB, PIO_PB21},
	{PIOB, PIO_PB22},
	{PIOB, PIO_PB23},
	{PIOB, PIO_PB24},
	{PIOB, PIO_PB25},
	{PIOB, PIO_PB26},
	{PIOB, PIO_PB27},
	{PIOB, PIO_PB28},
	{PIOB, PIO_PB29},
	{PIOB, PIO_PB30},
	{PIOB, PIO_PB31},
	{PIOC, PIO_PC0},
	{PIOC, PIO_PC1},
	{PIOC, PIO_PC2},
	{PIOC, PIO_PC3},
	{PIOC, PIO_PC4},
	{PIOC, PIO_PC5},
	{PIOC, PIO_PC6},
	{PIOC, PIO_PC7},
	{PIOC, PIO_PC8},
	{PIOC, PIO_PC9},
	{PIOC, PIO_PC10},
	{PIOC, PIO_PC11},
	{PIOC, PIO_PC12},
	{PIOC, PIO_PC13},
	{PIOC, PIO_PC14},
	{PIOC, PIO_PC15},
	{PIOC, PIO_PC16},
	{PIOC, PIO_PC17},
	{PIOC, PIO_PC18},
	{PIOC, PIO_PC19},
	{PIOC, PIO_PC20},
	{PIOC, PIO_PC21},
	{PIOC, PIO_PC22},
	{PIOC, PIO_PC23},
	{PIOC, PIO_PC24},
	{PIOC, PIO_PC25},
	{PIOC, PIO_PC26},
	{PIOC, PIO_PC27},
	{PIOC, PIO_PC28},
	{PIOC, PIO_PC29},
	{PIOC, PIO_PC30},
	{PIOC, PIO_PC30},
	{PIOD, PIO_PD0},
	{PIOD, PIO_PD1},
	{PIOD, PIO_PD2},
	{PIOD, PIO_PD3},
	{PIOD, PIO_PD4},
	{PIOD, PIO_PD5},
	{PIOD, PIO_PD6},
	{PIOD, PIO_PD7},
	{PIOD, PIO_PD8},
	{PIOD, PIO_PD9},
	{PIOD, PIO_PD10}
}; 

uint8_t gpio_is_valid(gpio_pin_t pin){
	return 1; 
}

void 		gpio_write_pin(gpio_pin_t pin, uint8_t value){
	/*if(value){
		gpio_set(pin); 
	} else {
		gpio_clear(pin); 
	}*/
	//if(!gpio_is_valid(pin)) return; 
	struct PinDecl *p = &_pins[pin]; 
	if(value != 0)
		p->pio->PIO_SODR = p->id; 
	else 
		p->pio->PIO_CODR = p->id; 
}

uint8_t 	gpio_read_pin(gpio_pin_t pin){
	if(!gpio_is_valid(pin)) return -1; 
	return (_pins[pin].pio->PIO_ODSR & _pins[pin].id)?1:0; 
}

void 		gpio_set_function(gpio_pin_t pin, uint8_t fun){
	EPioType type = PIO_INPUT; 
	switch(fun){
		case GP_OUTPUT: type = PIO_OUTPUT_1; break; 
		case GP_INPUT: type = PIO_INPUT; break; 
		case GP_AUX_A: type = PIO_PERIPH_A; break; 
		case GP_AUX_B: type = PIO_PERIPH_B; break; 
	}
	PIO_Configure(_pins[pin].pio, type, _pins[pin].id, PIO_DEFAULT); 
}

void 		gpio_set_pullup(gpio_pin_t pin, uint8_t pullup){
	
}

