/*
	Current sensor driver

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

	Special thanks to:
	* Davide Gironi, original implementation
*/

/*
 * get the current
 * current = (V - Vref/2) / sensitivity
 */

#include <arch/soc.h>

#include "acs712.h"

float acs712_read_current(uint8_t adc_chan,
	float sensitivity, float vcc_volt) {
	(void)(adc_chan); 
	uint16_t adc_value = adc0_read_immediate_ref(adc_chan, ADC_REF_AVCC_CAP_AREF);
	float volt = ((float)(adc_value) / (float)(65535)) * vcc_volt;
	return (volt - (vcc_volt / 2)) / sensitivity; 
}

