#include <arch/soc.h>

#define TIM_COUNT 8

// table for mapping pwm channels to timers
static const struct {
	TIM_TypeDef *tim; 
	int	id; 
	struct {
		GPIO_TypeDef *gpio; 
		int	gpio_pin; 
	} channels[4]; 
} _timers[TIM_COUNT] = {
	{.tim = TIM1, .id = RCC_APB2Periph_TIM1, 
		.channels = {
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_8, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_9, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_10, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_11, }
		}
	}, 
	{.tim = TIM2, .id = RCC_APB1Periph_TIM2, 
		.channels = {
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_0, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_1, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_2, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_3, }
		}
	},
	{.tim = TIM3, .id = RCC_APB1Periph_TIM3,
		.channels = {
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_4, }, // remapped
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_5, }, // remapped
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_0, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_1, }
		}
	},
	{.tim = TIM4, .id = RCC_APB1Periph_TIM4,
		.channels = {
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_6, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_7, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_8, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_9, }
		}
	},
	{.tim = TIM5, .id = RCC_APB1Periph_TIM5, 
		.channels = {
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_0, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_1, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_2, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_3, }
		}
	},
	{.tim = 0},
	{.tim = 0},
	{.tim = TIM8, .id = RCC_APB2Periph_TIM8, 
		.channels = {
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_6, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_7, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_8, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_9, }
		}
	}
}; 

void pwm_configure(pwm_channel_t chan, uint32_t def_width, uint32_t period){
	int tim_id = (chan >> 2) & 0x7; 
	if(tim_id > TIM_COUNT || tim_id < 0 || _timers[tim_id].tim == 0) return; 
	TIM_TypeDef *TIMx = _timers[tim_id].tim; 
	
	if(tim_id == 0 || tim_id == 7)
		RCC_APB2PeriphClockCmd(_timers[tim_id].id, ENABLE);
	else 
		RCC_APB1PeriphClockCmd(_timers[tim_id].id, ENABLE);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	TIM_OCInitTypeDef outputChannelInit;
	
	TIM_TimeBaseStructInit(&timerInitStructure); 
	
	timerInitStructure.TIM_Prescaler = F_CPU/1000000UL; // set 1us resolution
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = period;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMx, &timerInitStructure);
	
	TIM_OCStructInit(&outputChannelInit); 
	
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = def_width;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;


	switch((chan & 0x3)){
		case 0: 
			TIM_OC1Init(TIMx, &outputChannelInit);
			TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
		case 1: 
			TIM_OC2Init(TIMx, &outputChannelInit);
			TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
		case 2: 
			TIM_OC3Init(TIMx, &outputChannelInit);
			TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
		case 3: 
			TIM_OC4Init(TIMx, &outputChannelInit);
			TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
	}; 
	
	//TIM_ARRPreloadConfig(TIMx, ENABLE);
	TIM_CtrlPWMOutputs(TIMx, ENABLE);
	TIM_Cmd(TIMx, ENABLE);

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = _timers[tim_id].channels[chan & 0x3].gpio_pin;
	gpioStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(_timers[tim_id].channels[chan & 0x3].gpio, &gpioStructure);
	
	// remap tim3_ch1 and tim3_ch2 to pb4 and pb4
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
	
	//GPIO_PinRemapConfig(GPIO_PartialRemap_TIM1, ENABLE);
	
	/*
	TIM_TimeBaseStructure.TIM_Prescaler = 18; 
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 
	TIM_Cmd(TIM2, ENABLE); 
	
	// Start PWM Timer 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; 
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; 
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; 
	TIM_OCInitStructure.TIM_Pulse = 1000; 
	
	// THIS IS THE WIDTH in 1/8 us  PWM1 Mode configuration: TIM2 Channel1 
	TIM_OC1Init(TIM2, &TIM_OCInitStructure); 
	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable); // PWM1 Mode configuration: TIM2 Channel2 
	TIM_OC2Init(TIM2, &TIM_OCInitStructure); 
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable); // add OC3_ and OC4_ for 3rd and 4th channels
	TIM_ARRPreloadConfig(TIM2, ENABLE);
*/

	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_GPIOB, ENABLE);

	//GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM4);
	
}
/*
void pwm1_enable(void){
	TIM_OCInitTypeDef outputChannelInit = {0,};
	outputChannelInit.TIM_OCMode = TIM_OCMode_PWM1;
	outputChannelInit.TIM_Pulse = 100;
	outputChannelInit.TIM_OutputState = TIM_OutputState_Enable;
	outputChannelInit.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC2Init(TIM4, &outputChannelInit);
	TIM_OC2PreloadConfig(TIM4, TIM_OCPreload_Enable);

	GPIO_PinAFConfig(GPIOD, GPIO_PinSource13, GPIO_AF_TIM4);
}
*/
