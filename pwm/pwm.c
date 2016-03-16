/**
	PWM Subsystem

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

#include <kernel/list.h>
#include "pwm.h"

#if 0
static LIST_HEAD(_pwm_devices); 

void pwm_register_device(struct pwm_device *dev){
	list_add_tail(&dev->list, &_pwm_devices); 	
}

struct pwm_device *pwm_get_device(int number){
	return list_get_entry(&_pwm_devices, number, struct pwm_device, list); 
}
#endif
