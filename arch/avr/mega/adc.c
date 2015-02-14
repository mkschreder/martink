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

#include "adc.h"

uint8_t _adc_mode = ADC_MODE_AUTOMATIC;

//#if defined(CONFIG_ADC_MODE_AUTOMATIC)
uint16_t _adc_values[8];

ISR(ADC_vect){
	_adc_values[adc0_get_channel() & 0x07] = ADC;
	if(_adc_mode == ADC_MODE_AUTOMATIC){
		adc0_set_channel((adc0_get_channel() + 1) & 0x07); 
		adc0_start_conversion(); 
	}
}
//#endif
