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

struct pwm_device_info {
	uint16_t channels_available; 
	uint16_t channels_enabled; 
}; 

struct pwm_device; 
struct pwm_device_ops {
	void (*set_output)(struct pwm_device *dev, uint8_t chan, int enabled); 
	void (*set_period)(struct pwm_device *dev, uint8_t chan, int period); 
	void (*get_info)(struct pwm_device *dev, struct pwm_device_info *info);  
}; 

struct pwm_device {
	struct pwm_device_ops *ops; 
}; 

#define pwm_set_output(dev, chan, on) dev->ops->set_output(dev, chan, on)
#define pwm_set_period(dev, chan, on) dev->ops->set_period(dev, chan, on)

struct pwm_device *atmega_pwm_get_device(void); 

