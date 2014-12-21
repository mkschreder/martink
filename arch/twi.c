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

#define DEVICE_CAST(from, to) struct twi_device *to = container_of(from, struct twi_device, interface);  

void twi_init(uint8_t dev){
	switch(dev){
		case 0: twi0_init_default(); break;
#ifdef CONFIG_HAVE_TWI1
		case 1: twi1_init_default(); break;
#endif
	}
}

void 			_twi_stop(struct i2c_interface *self){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_stop(); break;
	}
}

uint32_t	_twi_write(struct i2c_interface *self, uint8_t adr, uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_start_write(adr, data, max_sz); return 0;
	}
	return PK_ERR_INVALID; 
}

uint32_t	_twi_read(struct i2c_interface *self, uint8_t adr, uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_start_read(adr, data, max_sz); return 0;
	}
	return PK_ERR_INVALID; 
}

void			_twi_sync(struct i2c_interface *self){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: while(twi0_busy()); break;
	}
}

uint16_t 	_twi_packets_available(struct i2c_interface *self){
	// for twi interfaces there is always data availbale because we clock it out of the slave. 
	return 1; 
}

uint8_t twi_get_interface(uint8_t id, struct twi_device *dev){
	if(id != 0) return 0;
	*dev = (struct twi_device) {
		.id = id,
		.interface = (struct i2c_interface) {
			.start_write = 	_twi_write,
			.start_read = 	_twi_read,
			.stop = 		_twi_stop, 
		}
	}; 
	return 1; 
}
