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

/*
static uint16_t 		_pwm_set_channel_duty(struct pwm_interface *self, uint8_t channel, uint16_t value_us){
	(void)(self); 
	switch(channel){
		case 0: return pwm0_set(value_us);
		case 1: return pwm1_set(value_us);
		// not available because timer1 is always used for timing tasks!
		//case 2: return pwm2_set(value_us);
		//case 3: return pwm3_set(value_us);
		case 4: return pwm4_set(value_us);
		case 5: return pwm5_set(value_us);
		default: return 0; 
	}
}

static uint16_t 		_pwm_set_channel_period(struct pwm_interface *self, uint8_t channel, uint16_t value_us){
	(void)(self); 
	(void)(channel); 
	(void)(value_us); 
	return 0; 
}

struct pwm_interface pwm_get_interface(void){
	return (struct pwm_interface){
		.set_channel_duty = _pwm_set_channel_duty,
		.set_channel_period = _pwm_set_channel_period
	}; 
}
*/

void initproc pwm_init(void){
	kdebug("PWM: starting channels: ");
#ifdef pwm0_enable
	pwm0_enable(); kdebug("pwm0 ");
#endif
#ifdef pwm1_enable
	pwm1_enable(); kdebug("pwm1 ");
#endif
#ifdef pwm2_enable
	pwm2_enable(); kdebug("pwm2 ");
#endif
#ifdef pwm3_enable
	pwm3_enable(); kdebug("pwm3 ");
#endif
#ifdef pwm4_enable
	pwm4_enable(); kdebug("pwm4 ");
#endif
#ifdef pwm5_enable
	pwm5_enable(); kdebug("pwm5 ");
#endif
	kdebug("\n");
}
