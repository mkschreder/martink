/**
	Fast ADC implementation for ATmega using macros
	
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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include <arch/soc.h>
#include <kernel/mt.h>

#include "adc.h"
#include "atmega_adc.h"

struct atmega_adc {
	mutex_t lock, ready; 
	uint16_t value; 

	struct adc_device device;  
}; 

static struct atmega_adc adc0; 

ISR(ADC_vect){
	adc0.value = ADC; 
	mutex_unlock_from_isr(&adc0.ready); 
	/*if(!_adc_data) return; 
	*_adc_data = ADC; 
	_adc_data = 0; */
	/*_adc_values[adc0_get_channel() & 0x07] = adc;
	if(_adc_mode == ADC_MODE_AUTOMATIC){
		adc0_set_channel((adc0_get_channel() + 1) & 0x07); 
		adc0_start_conversion(); 
	}*/
}

static uint16_t atmega_adc_read(struct adc_device *dev, uint8_t chan){
	struct atmega_adc *self = container_of(dev, struct atmega_adc, device); 
	mutex_lock(&self->lock); 

	adc0_set_channel(chan & 0x07); 
	adc0_start_conversion(); 

	mutex_lock(&self->ready); 
	mutex_unlock(&self->lock); 

	return self->value; 
}

static struct adc_ops atmega_adc_ops = {
	.read_channel = atmega_adc_read
}; 

static void __init atmega_adc_init(void){
	memset(&adc0, 0, sizeof(adc0)); 
	mutex_init(&adc0.lock); 
	mutex_init(&adc0.ready); 
	mutex_lock(&adc0.ready); 
	adc0.device.ops = &atmega_adc_ops; 
	adc0_init_default(); 
	adc_register_device(&adc0.device); 
}

