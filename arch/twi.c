/**
	A generic packet interface for the on board twi peripherals

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

#include "twi.h"

#define DEVICE_CAST(from, to) struct twi_device *to = container_of(from, struct twi_device, interface);  

static struct twi_device _twi[4] = {
	{.id = 0, .interface = 0},
	{.id = 1, .interface = 0},
	{.id = 2, .interface = 0},
	{.id = 3, .interface = 0}
}; 

static int16_t 			_twi_stop(i2c_dev_t self){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: return twi0_stop(); break;
	}
	return -1; 
}

static uint32_t	_twi_write(i2c_dev_t self, uint8_t adr, const uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_start_write(adr, data, max_sz); return 0;
	}
	return PK_ERR_INVALID; 
}

static uint32_t	_twi_read(i2c_dev_t self, uint8_t adr, uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_start_read(adr, data, max_sz); return 0;
	}
	return PK_ERR_INVALID; 
}

static void			_twi_wait(i2c_dev_t self, uint8_t addr){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_wait(addr); break;
	}
}

i2c_dev_t twi_get_interface(uint8_t id){
	uint8_t count = sizeof(_twi) / sizeof(_twi[0]); 
	if(id >= count) return 0; 
	
	static struct i2c_interface _if;
	_if = (struct i2c_interface) {
		.start_write = 	_twi_write,
		.start_read = 	_twi_read,
		.stop = 		_twi_stop, 
		.wait = _twi_wait
	};
	_twi[id].interface = &_if; 
	return &_twi[id].interface; 
}

void initproc twi_init(void){
	kdebug("TWI: starting interfaces: ");
#ifdef CONFIG_HAVE_TWI0
	twi0_init_default(); kdebug("twi0 ");
#endif
#ifdef CONFIG_HAVE_TWI1
	twi1_init_default(); kdebug("twi1 "); 
#endif
#ifdef CONFIG_HAVE_TWI2
	twi2_init_default(); kdebug("twi2 "); 
#endif
	kdebug("\n"); 
}
