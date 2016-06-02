/*
	Generic ADC subsystem for microcontrollers

	Copyright (C) 2016 Martin K. Schröder <mkschreder.uk@gmail.com>

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version. (Please read LICENSE file on special
	permission to include this software in signed images). 

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
*/

#pragma once

#include <kernel/types.h>
#include <kernel/list.h>

struct adc_device; 
struct adc_ops {
	uint16_t (*read_channel)(struct adc_device *dev, uint8_t chan); 
}; 

struct adc_device {
	struct list_head list; 
	struct adc_ops *ops; 
}; 

void adc_register_device(struct adc_device *dev); 
struct adc_device *adc_get_device(uint8_t number); 

static inline uint16_t adc_read_channel(struct adc_device *self, uint8_t chan){
	if(self && self->ops && self->ops->read_channel) return self->ops->read_channel(self, chan); 
	return 0; 
}
