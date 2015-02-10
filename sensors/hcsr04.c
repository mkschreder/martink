/*
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

	Github: https://github.com/mkschreder

	Contributors:
	* Martin K. Schröder - original driver
*/

/**
 * Device driver for HC-SR04 ultrasound module
 */

#include <stdio.h>
#include <inttypes.h>

#include <arch/soc.h>

#include "hcsr04.h"

//values for state
#define ST_IDLE 0
#define ST_PULSE_SENT 1
#define HCSR04_PULSE_TIMEOUT 1000000UL

static uint8_t hcsr04_send_pulse(struct hcsr04 *self)
{
	pio_write_pin(self->gpio, self->trigger_pin, 1);
	delay_us(10);
	pio_write_pin(self->gpio, self->trigger_pin, 0);
	
	self->state = ST_PULSE_SENT;
	self->pulse_timeout = timestamp_from_now_us(HCSR04_PULSE_TIMEOUT); 
	
	return 1;
}

void hcsr04_init(struct hcsr04 *self, pio_dev_t gpio,
	gpio_pin_t trigger_pin, gpio_pin_t echo_pin){
	self->gpio = gpio;
	self->trigger_pin = trigger_pin;
	self->echo_pin = echo_pin;

	pio_configure_pin(self->gpio, self->trigger_pin, GP_OUTPUT | GP_PULLUP);
	pio_configure_pin(self->gpio, self->echo_pin,
		GP_INPUT | GP_PCINT); 

	self->state = ST_IDLE;
	self->distance = -1;
	
	hcsr04_send_pulse(self); 
}

static uint8_t hcsr04_check_response(struct hcsr04 *self){
	timestamp_t t_up, t_down; 
	uint8_t status = pio_get_pin_status(self->gpio, self->echo_pin, &t_up, &t_down);
	if(status == GP_WENT_LOW){
		timestamp_t us = timestamp_ticks_to_us(t_down - t_up);
		// convert to cm 
		self->distance = (uint16_t)(us * 0.000001 * 34029.0f);
		self->state = ST_IDLE; 
		return 1;
	}
	return 0; 
}

int16_t hcsr04_read_distance_in_cm(struct hcsr04 *self)
{
	if(hcsr04_check_response(self) || self->state == ST_IDLE || timestamp_expired(self->pulse_timeout)){
		hcsr04_send_pulse(self); 
	}
	return self->distance;
}
