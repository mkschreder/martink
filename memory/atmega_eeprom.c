/**
	AVR interrupt based EEPROM driver
	
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


#include <arch/soc.h>
#include <avr/eeprom.h>
#include <kernel/mt.h>
#include "memory.h"

#define eeprom_set_address(address) ( EEAR = (address))
#define eeprom_interrupt_on() (EECR |= _BV(EERIE))
#define eeprom_interrupt_off() (EECR &= ~_BV(EERIE))
#define eeprom_wait_ready() while(EECR & _BV(EEPE)) 
#define eeprom_set_data(data) do { EEDR = data; } while(0); 
#define eeprom_get_data() (EEDR)
#define eeprom_start_write() { cli(); EECR |= _BV(EEMPE); EECR |= _BV(EEPE); sei(); }
#define eeprom_start_read() { EECR |= _BV(EERE); }

struct atmega_eeprom {
	struct memory_device device; 
	mutex_t lock; 
	sem_t ready; 
	const char *wr_buf; 
	uint16_t wr_size, wr_count, offset; 
}; 

static struct atmega_eeprom eeprom; 

ISR(EE_READY_vect){
	if(eeprom.wr_buf && eeprom.wr_count < eeprom.wr_size) {
		eeprom_set_address(eeprom.offset + eeprom.wr_count); 
		eeprom_set_data(eeprom.wr_buf[eeprom.wr_count]); 
		eeprom_start_write(); 
		eeprom.wr_count++; 
		return; 
	}
	eeprom_interrupt_off(); 
	sem_give_from_isr(&eeprom.ready); 
}

static int atmega_eeprom_read(struct memory_device *dev, size_t offset, char *data, size_t size){
	if(offset > size) return -EINVAL; 
	mutex_lock(&eeprom.lock); 
	eeprom_wait_ready(); 
	for(size_t c = offset; c < offset + size; c++){
		eeprom_set_address(c); 
		eeprom_start_read(); 
		data[c - offset] = eeprom_get_data(); 
	}
	mutex_unlock(&eeprom.lock); 
	return 0;
}

static int atmega_eeprom_write(struct memory_device *dev, size_t offset, const char *data, size_t size){
	mutex_lock(&eeprom.lock); 
	eeprom_wait_ready(); 
	eeprom_interrupt_off(); 
	eeprom_set_address(offset); 
	eeprom.wr_buf = data; 
	eeprom.wr_size = size; 
	eeprom.wr_count = 0; 
	eeprom.offset = offset; 
	eeprom_interrupt_on(); 
	eeprom_start_write(); 
	// writes take 26000 cycles (3.3ms). Let other things run.
	sem_take(&eeprom.ready); 
	mutex_unlock(&eeprom.lock); 
	return size; 
}

static struct memory_device_ops eeprom_ops = {
	.read = atmega_eeprom_read, 
	.write = atmega_eeprom_write
}; 

struct memory_device *atmega_eeprom_get(void){
	memset(&eeprom, 0, sizeof(eeprom)); 
	mutex_init(&eeprom.lock); 
	sem_init(&eeprom.ready, 1, 0); 
	eeprom.device.ops = &eeprom_ops; 
	return &eeprom.device; 
}

