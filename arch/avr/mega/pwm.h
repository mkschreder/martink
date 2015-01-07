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

#pragma once

#include "../../../util.h"

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

#define pwm0_set(speed) (\
	timer0_set_channel_a(map(speed, 0, 2000, 0, 127))\
)

#define pwm1_enable() (\
	timer0_mode(TIM0_MODE_FASTPWM_TOPMAX),\
	timer0_outb_mode(TIM0_OUTB_MODE_CLEAR),\
	timer0_set_clock(TIM0_CLOCK_DIV256)\
)

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

#define pwm4_set(speed) (\
	timer2_set_channel_a(map(speed, 0, 2000, 0, 127))\
)

#define pwm5_enable() (\
	timer2_mode(TIM2_MODE_FASTPWM_TOPMAX),\
	timer2_outb_mode(TIM2_OUTB_MODE_CLEAR),\
	timer2_set_clock(TIM2_CLOCK_DIV256)\
)

#define pwm5_set(speed) (\
	timer2_set_channel_b(map(speed, 0, 2000, 0, 127))\
)
