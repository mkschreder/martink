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

	References:
  + Using DS18B20 digital temperature sensor on AVR microcontrollers
    by Gerard Marull Paretas, 2007
    http://teslabs.com/openplayer/docs/docs/other/ds18b20_pre1.pdf
*/

#ifndef DS18B20_H_
#define DS18B20_H_

//setup connection
#define DS18B20_PORT PORTC
#define DS18B20_DDR DDRC
#define DS18B20_PIN PINC
#define DS18B20_DQ PC0

//commands
#define DS18B20_CMD_CONVERTTEMP 0x44
#define DS18B20_CMD_RSCRATCHPAD 0xbe
#define DS18B20_CMD_WSCRATCHPAD 0x4e
#define DS18B20_CMD_CPYSCRATCHPAD 0x48
#define DS18B20_CMD_RECEEPROM 0xb8
#define DS18B20_CMD_RPWRSUPPLY 0xb4
#define DS18B20_CMD_SEARCHROM 0xf0
#define DS18B20_CMD_READROM 0x33
#define DS18B20_CMD_MATCHROM 0x55
#define DS18B20_CMD_SKIPROM 0xcc
#define DS18B20_CMD_ALARMSEARCH 0xec

//decimal conversion table
#define DS18B20_DECIMALSTEPS_9BIT  5000 //0.5
#define DS18B20_DECIMALSTEPS_10BIT 2500 //0.25
#define DS18B20_DECIMALSTEPS_11BIT 1250 //0.125
#define DS18B20_DECIMALSTEPS_12BIT 625  //0.0625
#define DS18B20_DECIMALSTEPS DS18B20_DECIMALSTEPS_12BIT

struct ds18b20 {
	pio_dev_t gpio;
	gpio_pin_t data_pin;
};

void ds18b20_init(struct ds18b20 *self,
	pio_dev_t gpio, gpio_pin_t data_pin); 
double ds18b20_read_temperature(struct ds18b20 *self);

#endif
