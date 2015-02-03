/*
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

#include "ds18b20.h"

#define pin_write(value) pio_write_pin(self->gpio, self->data_pin, value)
#define pin_dir(dir) pio_configure_pin(self->gpio, self->data_pin, dir)
#define pin_read() pio_read_pin(self->gpio, self->data_pin)

static uint8_t ds18b20_reset(struct ds18b20 *self) {
	uint8_t i;

	pin_write(0);
	pin_dir(GP_OUTPUT);
	delay_us(480);
	
	//release line and wait for 60uS
	pin_dir(GP_INPUT); 
	delay_us(60);

	//get value and wait 420us
	i = pin_read(); 
	delay_us(420);

	return i;
}

/*
 * write one bit
 */
static void ds18b20_writebit(struct ds18b20 *self, uint8_t bit){
	// low for 1us
	pin_write(0);
	pin_dir(GP_OUTPUT);
	delay_us(1);
	
	//if we want to write 1, release the line (if not will keep low)
	if(bit)
		pin_dir(GP_INPUT); 

	//wait 60uS and release the line
	delay_us(60);
	pin_dir(GP_INPUT);
}

/*
 * read one bit
 */
static uint8_t ds18b20_readbit(struct ds18b20 *self){
	uint8_t bit=0;
	//low for 1uS
	pin_write(0); 
	pin_dir(GP_OUTPUT); 
	delay_us(1);

	//release line and wait for 14uS
	pin_dir(GP_INPUT); 
	delay_us(14);

	//read the value
	if(pin_read())
		bit=1;

	//wait 45uS and return read value
	delay_us(45);

	return bit;
}

/*
 * write one byte
 */
static void ds18b20_writebyte(struct ds18b20 *self, uint8_t byte){
	uint8_t i=8;
	while(i--){
		ds18b20_writebit(self, byte&1);
		byte >>= 1;
	}
}

/*
 * read one byte
 */
static uint8_t ds18b20_readbyte(struct ds18b20 *self){
	uint8_t i=8, n=0;
	while(i--){
		n >>= 1;
		n |= (ds18b20_readbit(self)<<7);
	}
	return n;
}


void ds18b20_init(struct ds18b20 *self,
	pio_dev_t gpio, gpio_pin_t data_pin){
	self->gpio = gpio; 
	self->data_pin = data_pin; 
}

/*
 * get temperature
 */
double ds18b20_read_temperature(struct ds18b20 *self) {
	uint8_t temperature[2];
	int8_t digit;
	uint16_t decimal;
	double retd = 0;

	ds18b20_reset(self); //reset
	ds18b20_writebyte(self, DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(self, DS18B20_CMD_CONVERTTEMP); //start temperature conversion

	while(!ds18b20_readbit(self)); //wait until conversion is complete

	ds18b20_reset(self); //reset
	ds18b20_writebyte(self, DS18B20_CMD_SKIPROM); //skip ROM
	ds18b20_writebyte(self, DS18B20_CMD_RSCRATCHPAD); //read scratchpad

	//read 2 byte from scratchpad
	temperature[0] = ds18b20_readbyte(self);
	temperature[1] = ds18b20_readbyte(self);

	ds18b20_reset(self); //reset

	//store temperature integer digits
	digit = temperature[0]>>4;
	digit |= (temperature[1]&0x7)<<4;

	//store temperature decimal digits
	decimal = temperature[0]&0xf;
	decimal *= DS18B20_DECIMALSTEPS;

	//compose the double temperature value and return it
	retd = digit + decimal * 0.0001;

	return retd;
}

