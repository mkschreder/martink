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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <arch/soc.h>

#include "bmp085.h"

//registers
#define BMP085_REGAC1 0xAA
#define BMP085_REGAC2 0xAC
#define BMP085_REGAC3 0xAE
#define BMP085_REGAC4 0xB0
#define BMP085_REGAC5 0xB2
#define BMP085_REGAC6 0xB4
#define BMP085_REGB1 0xB6
#define BMP085_REGB2 0xB8
#define BMP085_REGMB 0xBA
#define BMP085_REGMC 0xBC
#define BMP085_REGMD 0xBE
#define BMP085_REGCONTROL 0xF4 //control
#define BMP085_REGCONTROLOUTPUT 0xF6 //output 0xF6=MSB, 0xF7=LSB, 0xF8=XLSB
#define BMP085_REGREADTEMPERATURE 0x2E //read temperature
#define BMP085_REGREADPRESSURE 0x34 //read pressure

//modes
#define BMP085_MODEULTRALOWPOWER 0 //oversampling=0, internalsamples=1, maxconvtimepressure=4.5ms, avgcurrent=3uA, RMSnoise_hPA=0.06, RMSnoise_m=0.5
#define BMP085_MODESTANDARD 1 //oversampling=1, internalsamples=2, maxconvtimepressure=7.5ms, avgcurrent=5uA, RMSnoise_hPA=0.05, RMSnoise_m=0.4
#define BMP085_MODEHIGHRES 2 //oversampling=2, internalsamples=4, maxconvtimepressure=13.5ms, avgcurrent=7uA, RMSnoise_hPA=0.04, RMSnoise_m=0.3
#define BMP085_MODEULTRAHIGHRES 3 //oversampling=3, internalsamples=8, maxconvtimepressure=25.5ms, avgcurrent=12uA, RMSnoise_hPA=0.03, RMSnoise_m=0.25

//autoupdate temperature enabled
#define BMP085_AUTOUPDATETEMP 1 //autoupdate temperature every read

//setup parameters
#define BMP085_MODE BMP085_MODESTANDARD //define a mode
#define BMP085_UNITPAOFFSET 0 //define a unit offset (pa)
#define BMP085_UNITMOFFSET 0 //define a unit offset (m)

//avarage filter
#define BMP085_FILTERPRESSURE 1 //avarage filter for pressure

/*
static int self->regac1, self->regac2, self->regac3, self->regb1, self->regb2, self->regmb, self->regmc, self->regmd;
static unsigned int self->regac4, self->regac5, self->regac6;
static long bmp085_rawtemperature, bmp085_rawpressure;
*/

static void bmp085_writemem(struct bmp085 *self, uint8_t reg, uint8_t value) {
	//uart0_printf("BMP: writing reg %d: %d .. ", reg, value);
	uint8_t buf[2] = {reg, value};
	i2c_start_write(self->i2c, self->addr, buf, 2);
	i2c_stop(self->i2c); 
}

/*
 * i2c read
 */
static void bmp085_readmem(struct bmp085 *self, uint8_t reg, uint8_t *buff, uint8_t bytes) {
	i2c_start_write(self->i2c, self->addr, &reg, 1);
	delay_us(10);
	i2c_start_read(self->i2c, self->addr, buff, bytes);
	i2c_stop(self->i2c); 
}


#if BMP085_FILTERPRESSURE == 1
#define BMP085_AVARAGECOEF 21
/*
static long k[BMP085_AVARAGECOEF];
long bmp085_avaragefilter(long input) {
	uint8_t i=0;
	long sum=0;
	for (i=0; i<BMP085_AVARAGECOEF; i++) {
		k[i] = k[i+1];
	}
	k[BMP085_AVARAGECOEF-1] = input;
	for (i=0; i<BMP085_AVARAGECOEF; i++) {
		sum += k[i];
	}
	return (sum /BMP085_AVARAGECOEF) ;
}*/
#endif

/*
 * read calibration registers
 */
static void bmp085_getcalibration(struct bmp085 *self) {
	uint8_t buff[4];
	memset(buff, 0, sizeof(buff));

	bmp085_readmem(self, BMP085_REGAC1, buff, 2);
	self->regac1 = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGAC2, buff, 2);
	self->regac2 = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGAC3, buff, 2);
	self->regac3 = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGAC4, buff, 2);
	self->regac4 = ((unsigned int)buff[0] <<8 | ((unsigned int)buff[1]));
	bmp085_readmem(self, BMP085_REGAC5, buff, 2);
	self->regac5 = ((unsigned int)buff[0] <<8 | ((unsigned int)buff[1]));
	bmp085_readmem(self, BMP085_REGAC6, buff, 2);
	self->regac6 = ((unsigned int)buff[0] <<8 | ((unsigned int)buff[1]));
	bmp085_readmem(self, BMP085_REGB1, buff, 2);
	self->regb1 = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGB2, buff, 2);
	self->regb2 = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGMB, buff, 2);
	self->regmb = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGMC, buff, 2);
	self->regmc = ((int)buff[0] <<8 | ((int)buff[1]));
	bmp085_readmem(self, BMP085_REGMD, buff, 2);
	self->regmd = ((int)buff[0] <<8 | ((int)buff[1]));
}

/*
 * get raw temperature as read by registers, and do some calculation to convert it
 */
static long bmp085_getrawtemperature(struct bmp085 *self) {
	uint8_t buff[2];
	memset(buff, 0, sizeof(buff));
	long ut,x1,x2;

	//read raw temperature
	bmp085_writemem(self, BMP085_REGCONTROL, BMP085_REGREADTEMPERATURE);
	delay_us(5000L); // min. 4.5ms read Temp delay
	bmp085_readmem(self, BMP085_REGCONTROLOUTPUT, buff, 2);
	ut = ((long)buff[0] << 8 | ((long)buff[1])); //uncompensated temperature value

	//calculate raw temperature
	x1 = ((long)ut - self->regac6) * self->regac5 >> 15;
	x2 = ((long)self->regmc << 11) / (x1 + self->regmd);
	return x1 + x2;
}

/*
 * get raw pressure as read by registers, and do some calculation to convert it
 */
static long bmp085_getrawpressure(struct bmp085 *self) {
	uint8_t buff[3];
	memset(buff, 0, sizeof(buff));
	long up,x1,x2,x3,b3,b6,p;
	unsigned long b4,b7;

	long temperature = bmp085_getrawtemperature(self);
	
	//read raw pressure
	bmp085_writemem(self, BMP085_REGCONTROL, BMP085_REGREADPRESSURE+(BMP085_MODE << 6));
	delay_us((2 + (3<<BMP085_MODE)) * 1000L);
	bmp085_readmem(self, BMP085_REGCONTROLOUTPUT, buff, 3);
	up = ((((long)buff[0] <<16) | ((long)buff[1] <<8) | ((long)buff[2])) >> (8-BMP085_MODE)); // uncompensated pressure value

	//calculate raw pressure
	b6 = temperature - 4000;
	x1 = (self->regb2* (b6 * b6) >> 12) >> 11;
	x2 = (self->regac2 * b6) >> 11;
	x3 = x1 + x2;
	b3 = (((((long)self->regac1) * 4 + x3) << BMP085_MODE) + 2) >> 2;
	x1 = (self->regac3 * b6) >> 13;
	x2 = (self->regb1 * ((b6 * b6) >> 12)) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (self->regac4 * (uint32_t)(x3 + 32768)) >> 15;
	b7 = ((uint32_t)up - b3) * (50000 >> BMP085_MODE);
	p = b7 < 0x80000000 ? (b7 << 1) / b4 : (b7 / b4) << 1;
	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;
	return p + ((x1 + x2 + 3791) >> 4);
}

float bmp085_read_temperature(struct bmp085 *self) {
	long temperature = bmp085_getrawtemperature(self);
	return ((temperature + 8) / 16.4) / 10.0;
}

long bmp085_read_pressure(struct bmp085 *self) {
	long pressure = bmp085_getrawpressure(self);
	return (pressure + BMP085_UNITPAOFFSET);
}

float bmp085_read_altitude(struct bmp085 *self) {
	long pressure = bmp085_getrawpressure(self);
	return ((1 - pow(pressure/(double)101325, 0.1903 )) / 0.0000225577) + BMP085_UNITMOFFSET; 
}

void bmp085_init(struct bmp085 *self, i2c_dev_t i2c, uint8_t addr) {
	self->i2c = i2c;
	self->addr = addr; 
	bmp085_getcalibration(self); //get calibration data
}
