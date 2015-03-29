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
	* this library is a porting of the bmp085driver 0.4 ardunio library
    http://code.google.com/p/bmp085driver/
	* Davide Gironi, original implementation
*/

#ifndef BMP085_H_
#define BMP085_H_

#include <stdio.h>

#include <kernel/dev/i2c.h>
#include <kernel/thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BMP085_ADDR (0x77<<1) //0x77 default I2C address

struct bmp085 {
	io_dev_t dev; // underlying register file device
	
	int32_t calib_data[11]; 
	//int16_t regac1, regac2, regac3, regb1, regb2, regmb, regmc, regmd;
	//uint16_t regac4, regac5, regac6;
	int32_t ut, up; 
	uint8_t buf[4]; // i2c buffer
	
	struct libk_thread kthread; 
	struct async_task task; 
	int 	count; 
	timestamp_t time; 
	uint8_t status; 
};

//functions
/// inits the device over the interface supplied 
void bmp085_init(struct bmp085 *self, io_dev_t i2c);
/// runs all background tasks for the bmp sensor
void bmp085_update(struct bmp085 *self); 
/// returns pressure 
long bmp085_read_pressure(struct bmp085 *self);
/// returns altitude
float bmp085_read_altitude(struct bmp085 *self);
/// returns temperature
float bmp085_read_temperature(struct bmp085 *self);

#ifdef __cplusplus
}
#endif
#endif
