/**
	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

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
	Github: https://github.com/mkschreder
*/
#include <arch/soc.h>
#include "gpio.h"
#include "stm32_gpio.h"

static const struct {
	GPIO_TypeDef *gpio; 
} _ports[] = {
	{
		.gpio = GPIOA
	}, 
	{
		.gpio = GPIOB
	}, 
	{
		.gpio = GPIOC
	},
	{
		.gpio = GPIOD
	}
}; 

static void _gpio_configure_pcint(gpio_pin_t p){
	GPIO_TypeDef *gpio = _ports[p >> 4].gpio; 
	uint32_t pin = (p & 0xf); 
	uint32_t bit = (1 << pin); 
	
	/* Tell system that you will use PD0 for EXTI_Line0 */
	//SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, pin /*EXTI_PinSource0*/);
	
	EXTI_InitTypeDef EXTI_InitStruct; 
	EXTI_InitStruct.EXTI_Line = bit; //EXTI_Line0;
	EXTI_InitStruct.EXTI_LineCmd = ENABLE;
	EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStruct);
	
	uint32_t irqn = EXTI0_IRQn; 
	switch(pin){
		case 0: irqn = EXTI0_IRQn; break; 
		case 1: irqn = EXTI1_IRQn; break; 
		case 2: irqn = EXTI2_IRQn; break;
		case 3: irqn = EXTI3_IRQn; break;
		case 4: irqn = EXTI4_IRQn; break;
		case 5: case 6: case 7: case 8: case 9: irqn = EXTI9_5_IRQn; break;
		case 10: case 11: case 12: case 13: case 15: irqn = EXTI15_10_IRQn; break;
	}
	uint32_t port = GPIO_PortSourceGPIOA; 
	if(gpio == GPIOA) port = GPIO_PortSourceGPIOA;
	else if(gpio == GPIOB) port = GPIO_PortSourceGPIOB;
	else if(gpio == GPIOC) port = GPIO_PortSourceGPIOC; 
	else if(gpio == GPIOD) port = GPIO_PortSourceGPIOD; 
	
	GPIO_EXTILineConfig(port, pin);
	
	NVIC_InitTypeDef NVIC_InitStruct; 
	NVIC_InitStruct.NVIC_IRQChannel = irqn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

static struct irq *_fn_exti_irq_arg = 0; 
static void (*_fn_exti_irq)(struct irq *self, uint32_t mask) = 0; 
static void EXTI_IRQ(void){
	//static volatile uint32_t _exti_state = 0; 
	uint32_t state = 0; 
	for(int c = 0; c < 16; c++){
		uint32_t line = (1 << c); 
		/* Make sure that interrupt flag is set */
		if (EXTI_GetITStatus(line)) {
			state |= line; 
			EXTI_ClearITPendingBit(line);
		}
	}
	if(_fn_exti_irq) _fn_exti_irq(_fn_exti_irq_arg, state); 
	//_exti_state = state; 
}
/*
static void _gpio_register_irq(struct irq *self, void (*irq)(struct irq *self, uint32_t mask)){
	_fn_exti_irq_arg = self; 
	_fn_exti_irq = irq; 
}
*/

void EXTI0_IRQHandler(void); 
void EXTI0_IRQHandler(void){
	EXTI_IRQ(); 
}

void EXTI1_IRQHandler(void); 
void EXTI1_IRQHandler(void){
	EXTI_IRQ(); 
}

void EXTI2_IRQHandler(void); 
void EXTI2_IRQHandler(void){
	EXTI_IRQ(); 
}

void EXTI3_IRQHandler(void); 
void EXTI3_IRQHandler(void){
	EXTI_IRQ(); 
}

void EXTI4_IRQHandler(void); 
void EXTI4_IRQHandler(void){
	EXTI_IRQ(); 
}

void EXTI9_5_IRQHandler(void); 
void EXTI9_5_IRQHandler(void){
	EXTI_IRQ(); 
}

void EXTI15_10_IRQHandler(void); 
void EXTI15_10_IRQHandler(void){
	EXTI_IRQ(); 
}

static void _stm32_gpio_configure(struct gpio_adapter *adapter, gpio_pin_t p, uint16_t flags){
	GPIO_InitTypeDef gpio;
	GPIO_StructInit(&gpio); // Reset init structure, if not it can cause issues...
	
	gpio.GPIO_Pin = (1 << (p & 0xf));
	
	if(flags & GP_INPUT){
		gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING; 
		if(flags & GP_PULLUP)
			gpio.GPIO_Mode = GPIO_Mode_IPU; 
		else if(flags & GP_PULLDOWN)
			gpio.GPIO_Mode = GPIO_Mode_IPD;
		else if(flags & GP_ANALOG)
			gpio.GPIO_Mode = GPIO_Mode_AIN; 
		if(flags & GP_PCINT)
			_gpio_configure_pcint(p); 
	} else if(flags & GP_OUTPUT){
		gpio.GPIO_Mode = GPIO_Mode_Out_PP; 
		gpio.GPIO_Speed = GPIO_Speed_50MHz;
		if(flags & GP_OPEN_DRAIN)
			gpio.GPIO_Mode = GPIO_Mode_Out_OD; 
		else if(flags & GP_PUSH_PULL)
			gpio.GPIO_Mode = GPIO_Mode_Out_PP; 
	}
	
	GPIO_Init(_ports[p >> 4].gpio, &gpio);
}

static uint8_t _stm32_gpio_read_pin(struct gpio_adapter *adapter, gpio_pin_t pin){
	if(pin >= GPIO_COUNT) return 0; 
	return (_ports[pin >> 4].gpio->IDR & (1 << (pin & 0xf))) != 0; 
}

static void _stm32_gpio_write_pin(struct gpio_adapter *adapter, gpio_pin_t pin, uint8_t val){
	if(pin >= GPIO_COUNT) return; 
	if(val) // the output is inverted for some reason?
		_ports[pin >> 4].gpio->BSRR = (1 << (pin & 0xf)); 
	else
		_ports[pin >> 4].gpio->BRR = (1 << (pin & 0xf)); 
}

static void _stm32_gpio_enable_irq(struct gpio_adapter *adapter, gpio_pin_t pin, uint8_t enabled){
	// FIXME: add stm32 gpio pin change interrupts
}

static void _stm32_gpio_register_irq(struct gpio_adapter *self, gpio_pin_t pin, void (*pcint_handler)(struct gpio_adapter *, void *data), void *data){

}

static struct gpio_adapter_ops _stm32_gpio_ops = {
	.write_pin = _stm32_gpio_write_pin, 
	.read_pin = _stm32_gpio_read_pin, 
	.configure = _stm32_gpio_configure,
	.register_irq = _stm32_gpio_register_irq,
	.enable_irq = _stm32_gpio_enable_irq
}; 

static struct gpio_adapter _stm32_gpio = {
	.ops = &_stm32_gpio_ops 
}; 

struct gpio_adapter *stm32_gpio_get_adapter(void){
	return &_stm32_gpio; 
}

static void __init _stm32_gpio_init_default(void){
	RCC_APB2PeriphClockCmd ( 
		RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | 
		RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE); 
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
 
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_StructInit(&GPIO_InitStructure); 
	/* Configure PA.13 (JTMS/SWDAT), PA.14 (JTCK/SWCLK) and PA.15 (JTDI) as
		 output push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* Disable the Serial Wire Jtag Debug Port SWJ-DP */
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_Disable, ENABLE);
	
	/* Configure PB.03 (JTDO) and PB.04 (JTRST) as output push-pull */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
	//GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	/* Enable clock for SYSCFG */
	//RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
}

