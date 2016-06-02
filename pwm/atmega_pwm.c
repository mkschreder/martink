/**
	This file is part of martink project.
	
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

#include <kernel/types.h>
#include <kernel/util.h>

///TODO: make it set prescaler values based on F_CPU

#define GPIO_PWM0 GPIO_OC0A
#define GPIO_PWM1 GPIO_OC0B
#define GPIO_PWM2 GPIO_OC1A
#define GPIO_PWM3 GPIO_OC1B
#define GPIO_PWM4 GPIO_OC2A
#define GPIO_PWM5 GPIO_OC2B

#define pwm0_enable() (\
	timer0_mode(TIM0_MODE_FASTPWM_TOPMAX),\
	timer0_outa_mode(TIM0_OUTA_MODE_CLEAR),\
	timer0_set_clock(TIM0_CLOCK_DIV256)\
)

#define pwm0_disable() timer0_outa_mode(TIM0_OUTA_MODE_NONE)

#define pwm0_set(speed) (\
	timer0_set_channel_a(map(speed, 0, 2000, 0, 127))\
)

#define pwm1_enable() (\
	timer0_mode(TIM0_MODE_FASTPWM_TOPMAX),\
	timer0_outb_mode(TIM0_OUTB_MODE_CLEAR),\
	timer0_set_clock(TIM0_CLOCK_DIV256)\
)

#define pwm1_disable() timer0_outb_mode(TIM0_OUTB_MODE_NONE)

#define pwm1_set(speed) (\
	timer0_set_channel_b(map(speed, 0, 2000, 0, 127))\
)

#if !defined(CONFIG_TIMESTAMP_COUNTER)
	// pwm 2 and 3 can only be used if timer1 is free (timestamp uses timer 1!)
	#define pwm2_enable() (\
		timer1_mode(TIM1_MODE_FASTPWM8_TOPMAX),\
		timer1_outa_mode(TIM1_OUTA_MODE_CLEAR),\
		timer1_set_clock(TIM1_CLOCK_DIV256)\
	)

	#define pwm2_set(speed) (\
		timer1_set_channel_a(map(speed, 0, 2000, 0, 250))\
	)

	#define pwm3_enable() (\
		timer1_mode(TIM1_MODE_FASTPWM8_TOPMAX),\
		timer1_outb_mode(TIM0_OUTB_MODE_CLEAR),\
		timer1_set_clock(TIM1_CLOCK_DIV256)\
	)

	#define pwm3_set(speed) (\
		timer1_set_channel_b(map(speed, 0, 2000, 0, 250))\
	)
#endif

#define pwm4_enable() (\
	timer2_mode(TIM2_MODE_FASTPWM_TOPMAX),\
	timer2_outa_mode(TIM2_OUTA_MODE_CLEAR),\
	timer2_set_clock(TIM2_CLOCK_DIV256)\
)

#define pwm4_disable() timer2_outa_mode(TIM2_OUTA_MODE_NONE)

#define pwm4_set(speed) (\
	timer2_set_channel_a(map(speed, 0, 2000, 0, 127))\
)

#define pwm5_enable() (\
	timer2_mode(TIM2_MODE_FASTPWM_TOPMAX),\
	timer2_outb_mode(TIM2_OUTB_MODE_CLEAR),\
	timer2_set_clock(TIM2_CLOCK_DIV256)\
)

#define pwm5_disable() timer2_outb_mode(TIM2_OUTB_MODE_NONE)

#define pwm5_set(speed) (\
	timer2_set_channel_b(map(speed, 0, 2000, 0, 127))\
)

#include <arch/avr/mega/time.h>
#include "pwm.h"

struct atmega_pwm {
	struct pwm_device device; 
}; 

static struct atmega_pwm mega_pwm; 

static void atmega_pwm_set_output(struct pwm_device *dev, uint8_t chan, int enabled){
	switch(chan){
		case 0: if(enabled) pwm0_enable(); else pwm0_disable(); break;  
		case 1: if(enabled) pwm1_enable(); else pwm1_disable(); break;  
		//case 2: pwm2_enable(); break;  
		//case 3: pwm3_enable(); break;  
		case 4: if(enabled) pwm4_enable(); else pwm4_disable(); break;  
		case 5: if(enabled) pwm5_enable(); else pwm5_disable(); break;  
	}
}; 

static void atmega_pwm_set_period(struct pwm_device *dev, uint8_t chan, int period){
	if(period == 0) { 
		atmega_pwm_set_output(dev, chan, 0); 
		return; 
	}

	atmega_pwm_set_output(dev, chan, 1); 

	switch(chan){
		case 0: pwm0_set(period); break;  
		case 1: pwm1_set(period); break;  
		//case 2: pwm2_set(period); break;  
		//case 3: pwm3_set(period); break;  
		case 4: pwm4_set(period); break;  
		case 5: pwm5_set(period); break;  
	}
}

static struct pwm_device_ops mega_pwm_ops = {
	.set_output = atmega_pwm_set_output, 
	.set_period = atmega_pwm_set_period
}; 

struct pwm_device *atmega_pwm_get_device(void){
	mega_pwm.device.ops = &mega_pwm_ops; 
	return &mega_pwm.device; 
}

