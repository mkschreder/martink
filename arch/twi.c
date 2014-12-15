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

#include <avr/io.h>

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

void 			_twi_begin(struct packet_interface *self){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_begin(); break;
	}
}

void 			_twi_end(struct packet_interface *self){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_end(); break;
	}
}

uint32_t	_twi_write(struct packet_interface *self, const uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: return twi0_start_write(data, max_sz); break;
	}
	return PK_ERR_INVALID; 
}

uint32_t	_twi_read(struct packet_interface *self, uint8_t *data, uint16_t max_sz){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: return twi0_start_read(data, max_sz); break;
	}
	return PK_ERR_INVALID; 
}

void			_twi_sync(struct packet_interface *self){
	DEVICE_CAST(self, dev);
	switch(dev->id){
		case 0: twi0_sync(); break;
	}
}

uint16_t 	_twi_packets_available(struct packet_interface *self){
	// for twi interfaces there is always data availbale because we clock it out of the slave. 
	return 1; 
}

void twi_get_interface(uint8_t id, struct twi_device *dev){
	*dev = (struct twi_device) {
		.id = id,
		.interface = (struct packet_interface) {
			.begin = 	_twi_begin, 
			.end = 		_twi_end, 
			.write = 	_twi_write,
			.read = 	_twi_read,
			.sync = 	_twi_sync,
			.packets_available = _twi_packets_available
		}
	}; 
}
