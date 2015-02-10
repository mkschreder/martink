/**
	8 bit i2c io expander driver

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

#include <arch/soc.h>

#include "pcf8574.h"

#define PCF8574_ADDRBASE (0x20) //device base address

#define PCF8574_MAXDEVICES 8 //max devices, depends on address (3 bit)
#define PCF8574_MAXPINS 8 //max pin per device

//#define PCF8574_LCD_DEVICEID 7

void pcf8574_init(struct pcf8574 *self, i2c_dev_t i2c, uint8_t device_id) {
	self->i2c = i2c;
	self->device_id = device_id;
	self->in_reg = self->out_reg = 0xff; 
}

static void _pcf8574_flush(struct pcf8574 *self){
	i2c_start_write(self->i2c,
		((PCF8574_ADDRBASE+self->device_id)<<1), &self->out_reg, 1);
	i2c_stop(self->i2c);
}

static void _pcf8574_read(struct pcf8574 *self){
	i2c_start_read(self->i2c,
		((PCF8574_ADDRBASE+self->device_id)<<1), &self->in_reg, 1);
	i2c_stop(self->i2c);
}

uint8_t pcf8574_write_word(struct pcf8574 *self, uint8_t data) {
	self->out_reg = data;
	_pcf8574_flush(self); 
	return 0; 
}

uint8_t pcf8574_write_pin(struct pcf8574 *self, uint8_t pin, uint8_t value) {
	pin &= 0x07; // this is an 8 bit device
	self->out_reg = (value)?(self->out_reg | (1 << pin)):(self->out_reg & ~(1 << pin));
	_pcf8574_flush(self); 
	return 0;
}

uint8_t pcf8574_read_word(struct pcf8574 *self) {
	_pcf8574_read(self);
	return self->in_reg; 
}

uint8_t pcf8574_read_pin(struct pcf8574 *self, uint8_t pin) {
	_pcf8574_read(self);
	return (self->in_reg & (1 << (pin & 0x07)))?1:0; 
}

