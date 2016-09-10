#include <arch/soc.h>
#include <gpio/gpio.h>
#if 0
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

void gpio_init_default(void){
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

void gpio_configure(gpio_pin_t p, uint16_t flags){
	//uint32_t pin = p; 
	//uint32_t reg = 0; 
	
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
	/*
	// set output 50mhz with push pull or floating input
	reg = (flags & GP_OUTPUT)?3:4; 
	
	// set open drain if it is selected (default push pull)
	reg |= ((flags & GP_OPEN_DRAIN)?1:0) << 2; 
	
	// set alternative function if selected 
	reg |= (((flags & GP_OUTPUT) && (flags & GP_AF))?1:0) << 3; 
	
	// set floating or pullup if input
	if(!(flags & GP_OUTPUT)) reg = (reg & 3) | (((flags & GP_PULLUP)?2:1) << 2); 
	
	// check for analog mode (resets the cnf bits)
	if(flags & GP_ANALOG) reg &= 3; 
	
	if((pin & 0xf) > 7) {
		uint32_t tmp = _ports[pin >> 4].gpio->CRH; 
		uint8_t shift = (((pin & 0xf) - 8) * 4); 
		_ports[pin >> 4].gpio->CRH = (tmp & ~(0xf << shift)) | (reg << shift); 
	} else {
		uint32_t tmp = _ports[pin >> 4].gpio->CRL; 
		uint8_t shift = ((pin & 0xf) * 4); 
		_ports[pin >> 4].gpio->CRL = (tmp & ~(0xf << shift)) | (reg << shift); 
	}*/
}

void gpio_write_pin(gpio_pin_t p, uint8_t val){
	uint32_t pin = p; 
	if(pin >= GPIO_COUNT) return; 
	if(val) // the output is inverted for some reason?
		_ports[pin >> 4].gpio->BSRR = (1 << (pin & 0xf)); 
	else
		_ports[pin >> 4].gpio->BRR = (1 << (pin & 0xf)); 
}

uint8_t gpio_read_pin(gpio_pin_t p){
	uint32_t pin = p; 
	if(pin >= GPIO_COUNT) return 0; 
	return (_ports[pin >> 4].gpio->IDR & (1 << (pin & 0xf))) != 0; 
}

uint16_t gpio_get_status(gpio_pin_t pin, timestamp_t *t_up, timestamp_t *t_down){
	*t_up = 0; 
	*t_down = 0; 
	(void)(pin); 
	return 0; 
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

void gpio_register_irq(struct irq *self, void (*irq)(struct irq *self, uint32_t mask)){
	_fn_exti_irq_arg = self; 
	_fn_exti_irq = irq; 
}

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
#endif
