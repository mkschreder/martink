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
#include <thread/pt.h>


static PT_THREAD(_acs712_update_thread(struct libk_thread *kthread, struct pt *pt)){
	struct acs712 *self = container_of(kthread, struct acs712, thread); 
	(void)self; 
	
	PT_BEGIN(pt); 
	/*
	while(1){
		PT_WAIT_UNTIL(pt, self->interval != 0 && timestamp_expired(self->time)); 
		
		PT_WAIT_UNTIL(pt, adc_aquire(self->adc_chan)); 
		
		adc_start_read(self->adc_chan, &self->raw_value); 
		
		PT_WAIT_WHILE(pt, adc_busy()); 
		
		self->time = timestamp_from_now_us(self->interval); 
	}*/
	PT_END(pt); 
}

void acs712_init(struct acs712 *self, uint8_t adc_chan, uint16_t read_interval_us){
	self->adc_chan = adc_chan; 
	self->interval = read_interval_us; 
	self->raw_value = 0; 
	self->time = timestamp_now(); 
	
	libk_create_thread(&self->thread, _acs712_update_thread, "acs0712"); 
}

float acs712_read_current(struct acs712 *self, float sensitivity, float vcc_volt) {
	if(vcc_volt == 0 || sensitivity == 0) return 0; 
	//uint16_t adc_value = adc0_read_immediate_ref(adc_chan, ADC_REF_AVCC_CAP_AREF);
	float volt = ((float)(self->raw_value) / (float)(65535)) * vcc_volt;
	return (volt - (vcc_volt / 2)) / sensitivity; 
}

