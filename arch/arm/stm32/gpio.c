#include <arch/soc.h>

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
