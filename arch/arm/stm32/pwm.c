/*
	PWM input and output driver. 

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

/*
	This code generates and reads pwm on one of the available timer channels
	
	Dependencies: systick timestamp system for timeout handling. 
*/

#include <arch/soc.h>

typedef enum {
	MODE_INPUT, 
	MODE_OUTPUT
} pin_mode_t; 

// table for mapping pwm channels to timers
static const struct _timer {
	TIM_TypeDef *tim; 
	int	id; 
	int irq; 
	struct {
		GPIO_TypeDef *gpio; 
		int	gpio_pin; 
	} channels[4]; 
} _timers[] = {
	{.tim = TIM1, .id = RCC_APB2Periph_TIM1, 
		.irq = TIM1_CC_IRQn, 
		.channels = {
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_8, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_9, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_10, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_11, }
		}
	}, 
	{.tim = TIM2, .id = RCC_APB1Periph_TIM2, 
		.irq = TIM2_IRQn, 
		.channels = {
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_0, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_1, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_2, },
			{.gpio = GPIOA, .gpio_pin = GPIO_Pin_3, }
		}
	},
	{.tim = TIM3, .id = RCC_APB1Periph_TIM3,
		.irq = TIM3_IRQn, 
		.channels = {
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_4, }, // remapped
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_5, }, // remapped
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_0, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_1, }
		}
	},
	{.tim = TIM4, .id = RCC_APB1Periph_TIM4,
		.irq = TIM4_IRQn, 
		.channels = {
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_6, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_7, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_8, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_9, }
		}
	},
	{.tim = TIM5, .id = RCC_APB1Periph_TIM5, 
		.irq = 0, 
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
		.irq = 0, 
		.channels = {
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_6, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_7, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_8, },
			{.gpio = GPIOB, .gpio_pin = GPIO_Pin_9, }
		}
	}
}; 

#define TIM_COUNT (sizeof(_timers)/sizeof(_timers[0]))

static struct PWM_State {
    uint8_t  state;
    uint32_t rise;
    uint32_t fall;
    uint32_t capture;
    timestamp_t us; 
    uint32_t def_value; 
} Inputs[8][4] = { { { 0, } }, };

enum {
	IC_DISABLED, 
	IC_RISE, 
	IC_FALL
}; 

const static int _channels[] = {TIM_Channel_1, TIM_Channel_2, TIM_Channel_3, TIM_Channel_4}; 
const static int _int_ids[] = {TIM_IT_CC1, TIM_IT_CC2, TIM_IT_CC2, TIM_IT_CC3}; 
	
static void _pwm_configure_oc_chan(TIM_TypeDef *TIMx, uint8_t chan, TIM_OCInitTypeDef *conf){
	switch(chan){
		case 0: 
			TIM_OC1Init(TIMx, conf);
			TIM_OC1PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
		case 1: 
			TIM_OC2Init(TIMx, conf);
			TIM_OC2PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
		case 2: 
			TIM_OC3Init(TIMx, conf);
			TIM_OC3PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
		case 3: 
			TIM_OC4Init(TIMx, conf);
			TIM_OC4PreloadConfig(TIMx, TIM_OCPreload_Enable);
			break; 
	}; 
}

void pwm_configure(pwm_channel_t chan, uint32_t def_width, uint32_t period){
	int chan_id = (chan & 0x3); 
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
	
	timerInitStructure.TIM_Prescaler = SystemCoreClock/1000000UL; // set 1us resolution
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

	_pwm_configure_oc_chan(TIMx, chan_id, &outputChannelInit); 
	
	// make arr register buffered
	//TIM_ARRPreloadConfig(TIMx, ENABLE);
	
	TIM_CtrlPWMOutputs(TIMx, ENABLE);
	TIM_Cmd(TIMx, ENABLE);

	GPIO_InitTypeDef gpioStructure;
	gpioStructure.GPIO_Pin = _timers[tim_id].channels[chan & 0x3].gpio_pin;
	gpioStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(_timers[tim_id].channels[chan & 0x3].gpio, &gpioStructure);
	
	// remap tim3_ch1 and tim3_ch2 to pb4 and pb5
	GPIO_PinRemapConfig(GPIO_PartialRemap_TIM3, ENABLE);
	
}

void pwm_configure_capture(pwm_channel_t chan, uint32_t def_value){
	//GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;

	int tim_id = (chan >> 2) & 0x7; 
	int chan_id = (chan & 0x3); 
	if(tim_id > TIM_COUNT || tim_id < 0 || _timers[tim_id].tim == 0 || _timers[tim_id].irq == 0) return; 
	
	const struct _timer *t = &_timers[tim_id]; 
	struct PWM_State *in = &Inputs[tim_id][chan_id]; 
	
	// clear input state
	in->rise = in->fall = in->capture = 0; 
	in->def_value = def_value; 
	in->state = IC_RISE; 
	
	TIM_TypeDef *TIMx = t->tim; 
	
	if(tim_id == 0 || tim_id == 7)
		RCC_APB2PeriphClockCmd(_timers[tim_id].id, ENABLE);
	else 
		RCC_APB1PeriphClockCmd(_timers[tim_id].id, ENABLE);

	TIM_TimeBaseInitTypeDef timerInitStructure;
	TIM_TimeBaseStructInit(&timerInitStructure); 
	
	timerInitStructure.TIM_Prescaler = SystemCoreClock/1000000UL; // set 1us resolution
	timerInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	timerInitStructure.TIM_Period = 4000;
	timerInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	timerInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIMx, &timerInitStructure);
	
	//TIM_DeInit(TIMx);

	TIM_ICInitStructure.TIM_Channel = _channels[chan_id];
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x7;
	TIM_ICInit(TIMx, &TIM_ICInitStructure);
	
	TIM_ITConfig(TIMx, _int_ids[chan_id], ENABLE);
	
	TIM_ITConfig(TIMx, TIM_IT_Update, ENABLE);
	
	/* TIM enable counter */
	TIM_Cmd(TIMx, ENABLE);
	
	GPIO_InitTypeDef gpioStructure;
	GPIO_StructInit(&gpioStructure); 
	
	gpioStructure.GPIO_Pin = _timers[tim_id].channels[chan_id].gpio_pin;
	gpioStructure.GPIO_Mode = GPIO_Mode_IPU; 
	//gpioStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
	gpioStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(_timers[tim_id].channels[chan_id].gpio, &gpioStructure);
	
	NVIC_InitStructure.NVIC_IRQChannel = _timers[tim_id].irq;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void pwm_write(pwm_channel_t chan, uint32_t width){
	TIM_TypeDef *TIMx = _timers[(chan >> 2) & 0x7].tim; 
	switch(chan & 0x3){
		case 0: 
			TIM_SetCompare1(TIMx, width);
			break; 
		case 1: 
			TIM_SetCompare2(TIMx, width);
			break; 
		case 2: 
			TIM_SetCompare3(TIMx, width);
			break; 
		case 3: 
			TIM_SetCompare4(TIMx, width);
			break; 
	}
	//_pwm_configure_oc_chan(_timers[(chan >> 2) & 0x7].tim, chan & 0x3, &outputChannelInit); 
}

uint32_t pwm_read(pwm_channel_t chan){
	return Inputs[(chan >> 2) & 0x7][chan & 0x3].capture; 
}

static inline void TIM_IRQHandler(int timer, TIM_TypeDef *TIMx){
	// check for one of the capture events
	for(int c = 0; c < sizeof(_int_ids) / sizeof(_int_ids[0]); c++){
		if (TIM_GetITStatus(TIMx, _int_ids[c]) != RESET) {
			TIM_ClearITPendingBit(TIMx, _int_ids[c]);
			
			struct PWM_State *in = &Inputs[timer][c]; 
			
			TIM_ICInitTypeDef TIM_ICInitStructure;
			TIM_ICStructInit(&TIM_ICInitStructure); 
			
			uint32_t Current = 0; 
			switch(c){
				case 0: Current = TIM_GetCapture1(TIMx); break; 
				case 1: Current = TIM_GetCapture2(TIMx); break; 
				case 2: Current = TIM_GetCapture3(TIMx); break; 
				case 3: Current = TIM_GetCapture4(TIMx); break; 
			}
			
			// save last capture time
			in->us = timestamp_now(); 
			
			if (in->state == IC_RISE) {
				in->rise = Current;
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Falling;
				TIM_ICInitStructure.TIM_Channel = _channels[c];
				TIM_ICInit(TIMx, &TIM_ICInitStructure);
				in->state = IC_FALL;
			} else if(in->state == IC_FALL){
				in->fall = Current;
				
				// compute period (must be done this way for correct result)
				if (in->fall > in->rise) {
					in->capture = (in->fall - in->rise);
				} else {
					in->capture = ((TIMx->ARR - in->rise) + in->fall);
				}
				
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
				TIM_ICInitStructure.TIM_Channel = _channels[c];
				TIM_ICInit(TIMx, &TIM_ICInitStructure);
				in->state = IC_RISE;
			}
		}
	}
	// check for overflow event
	if(TIM_GetITStatus(TIMx, TIM_IT_Update) != RESET){
		TIM_ClearITPendingBit(TIMx, TIM_IT_Update);
		for(int c = 0; c < 4; c++){
			struct PWM_State *in = &Inputs[timer][c]; 
			
			// skip disabled input channels
			if(Inputs[timer][c].state == IC_DISABLED) continue; 
			
			if(timestamp_expired(in->us + timestamp_us_to_ticks(100000))){
				in->rise = in->fall = 0; 
				in->capture = in->def_value; 
				
				// reset the polarity to rising and state to 0
				TIM_ICInitTypeDef TIM_ICInitStructure;
				TIM_ICStructInit(&TIM_ICInitStructure); 
				TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
				TIM_ICInitStructure.TIM_Channel = _channels[c];
				TIM_ICInit(TIMx, &TIM_ICInitStructure);
				
				in->state = IC_RISE; 
			}
		} 
	}
}

void TIM1_UP_IRQHandler(void){
	TIM_IRQHandler(0, TIM1); 
}

void TIM1_CC_IRQHandler(void){
	TIM_IRQHandler(0, TIM1); 
}

void TIM2_IRQHandler(void){
	TIM_IRQHandler(1, TIM2); 
}

void TIM3_IRQHandler(void){
	TIM_IRQHandler(2, TIM3); 
}

void TIM4_IRQHandler(void){
	TIM_IRQHandler(3, TIM4); 
}
