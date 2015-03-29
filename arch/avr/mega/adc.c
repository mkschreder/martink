/**
	Fast ADC implementation for ATmega using macros

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

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#include <arch/soc.h>

#include "adc.h"

void avr_adc_init(struct avr_adc *self){
	(void)self; 
	
}

ASYNC(int, adc_t, avr_adc_open){
	ASYNC_BEGIN(); 
	
	ASYNC_END(0); 
}
 
ASYNC(int, adc_t, avr_adc_close){
	ASYNC_BEGIN(); 
	ASYNC_END(0); 
}

ASYNC(uint16_t, adc_t, avr_adc_read, uint8_t channel){
	ASYNC_BEGIN(); 
	
	AWAIT(!adc0_conversion_in_progress()); 
	 
	adc0_set_channel(channel & 0x07); 
	adc0_start_conversion(); 
	
	AWAIT(!adc0_conversion_in_progress()); 
	
	ASYNC_END(ADC); 
}
/*
static volatile uint16_t *_adc_data = 0; 

uint8_t adc_aquire(uint8_t chan){
	if(chan > 8 || _adc_data) return 0; 
	return 1; 
}

void adc_release(void){
	// in this case, do nothing
}

uint8_t adc_busy(void){
	return adc0_conversion_in_progress(); 
}
*/
ISR(ADC_vect){
	/*if(!_adc_data) return; 
	*_adc_data = ADC; 
	_adc_data = 0; */
	/*_adc_values[adc0_get_channel() & 0x07] = adc;
	if(_adc_mode == ADC_MODE_AUTOMATIC){
		adc0_set_channel((adc0_get_channel() + 1) & 0x07); 
		adc0_start_conversion(); 
	}*/
}
//#endif
