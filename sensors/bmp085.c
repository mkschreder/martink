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

#define BMP085_STATUS_READY 1

static long bmp085_getrawtemperature(struct bmp085 *self) {
	long x1,x2;

	//calculate raw temperature
	x1 = ((int32_t)self->ut - self->regac6) * self->regac5 >> 15;
	x2 = ((long)self->regmc << 11) / (x1 + self->regmd);
	return x1 + x2;
}

static long bmp085_getrawpressure(struct bmp085 *self) {
	long x1,x2,x3,b3,b6,p;
	unsigned long b4,b7;

	long temperature = bmp085_getrawtemperature(self);
	
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
	b7 = ((uint32_t)self->up - b3) * (50000 >> BMP085_MODE);
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

static int _bmp085_read_reg(struct bmp085 *self, uint8_t reg, uint8_t width){
	if(!blk_open(self->dev)) return -1; 
	blk_seek(self->dev, reg, SEEK_SET); 
	blk_transfer_start(&self->tr, self->dev, self->buf, width, IO_READ); 
	return 1; 
}

static int _bmp085_write_reg8(struct bmp085 *self, uint8_t reg, uint8_t value){
	if(!blk_open(self->dev)) return -1; 
	blk_seek(self->dev, reg, SEEK_SET); 
	self->buf[0] = value; 
	blk_transfer_start(&self->tr, self->dev, self->buf, 1, IO_WRITE); 
	return 1; 
}

static uint8_t _bmp085_done(struct bmp085 *self){
	if(blk_transfer_result(&self->tr) == TR_BUSY) return 0; 
	blk_close(self->dev); 
	return 1; // TODO handle errors
}

#define REG_VALUE16() (((long)self->buf[0] <<8) | ((long)self->buf[1]))
#define REG_VALUEUINT() (((unsigned int)self->buf[0] <<8) | ((unsigned int)self->buf[1]))
#define REG_VALUE24() (((long)self->buf[0] << 16) | ((long)self->buf[1] << 8) | ((long)self->buf[2]))

#define READ_REG16(reg) PT_WAIT_UNTIL(thr, _bmp085_read_reg(self, reg, 2) > 0)
#define WAIT_REG16(target) do { PT_WAIT_UNTIL(thr, _bmp085_done(self)); target = REG_VALUE16(); } while(0)
#define WAIT_REGUINT(target) do { PT_WAIT_UNTIL(thr, _bmp085_done(self)); target = REG_VALUEUINT(); } while(0)
#define READ_REG24(reg) PT_WAIT_UNTIL(thr, _bmp085_read_reg(self, reg, 3) > 0)
#define WAIT_REG24(target) do { PT_WAIT_UNTIL(thr, _bmp085_done(self)); target = REG_VALUE24(); } while(0)
#define WRITE_REG8(reg, value) PT_WAIT_UNTIL(thr, _bmp085_write_reg8(self, reg, value) > 0)
#define WAIT() do { PT_WAIT_UNTIL(thr, _bmp085_done(self)); } while(0)
#define TIMEOUT(t) do { self->time = timestamp_from_now_us(t); \
		PT_WAIT_UNTIL(thr, timestamp_expired(self->time)); } while(0); 

PT_THREAD(_bmp085_thread(struct libk_thread *kthread, struct pt *thr)); 
PT_THREAD(_bmp085_thread(struct libk_thread *kthread, struct pt *thr)){
	struct bmp085 *self = container_of(kthread, struct bmp085, thread); 
	
	PT_BEGIN(thr); 
	
	READ_REG16(BMP085_REGAC1); 
	WAIT_REG16(self->regac1); 
	READ_REG16(BMP085_REGAC2); 
	WAIT_REG16(self->regac2); 
	READ_REG16(BMP085_REGAC3); 
	WAIT_REG16(self->regac3); 
	READ_REG16(BMP085_REGAC4); 
	WAIT_REGUINT(self->regac4); 
	READ_REG16(BMP085_REGAC5); 
	WAIT_REGUINT(self->regac5); 
	READ_REG16(BMP085_REGAC6); 
	WAIT_REGUINT(self->regac6); 
	READ_REG16(BMP085_REGB1); 
	WAIT_REG16(self->regb1); 
	READ_REG16(BMP085_REGB2); 
	WAIT_REG16(self->regb2); 
	READ_REG16(BMP085_REGMB); 
	WAIT_REG16(self->regmb); 
	READ_REG16(BMP085_REGMC); 
	WAIT_REG16(self->regmc); 
	READ_REG16(BMP085_REGMD); 
	WAIT_REG16(self->regmd); 
	
	self->status |= BMP085_STATUS_READY; 
	
	while(1){
		// read uncompensated temperature
		WRITE_REG8(BMP085_REGCONTROL, BMP085_REGREADTEMPERATURE); 
		WAIT(); 
		
		// min. 4.5ms read Temp delay
		TIMEOUT(5000); 
		
		READ_REG16(BMP085_REGCONTROLOUTPUT); 
		WAIT_REG16(self->ut); 
		
		// read uncompensated pressure
		WRITE_REG8(BMP085_REGCONTROL, BMP085_REGREADPRESSURE+(BMP085_MODE << 6)); 
		WAIT(); 
		
		TIMEOUT((2 + (3<<BMP085_MODE)) * 1000L); 
		
		READ_REG24(BMP085_REGCONTROLOUTPUT); 
		WAIT_REG24(self->up); 
		
		//printf("P: %04x%04x ", (int16_t)(self->up >> 16), (int16_t)self->up); 
		//printf("T: %04x%04x\n", (int16_t)(self->ut >> 16), (int16_t)self->ut); 
		
		//PT_SPAWN(thr, bthr, i2c_read_reg_thread(self->i2c, bthr, self->addr, BMP085_REGCONTROLOUTPUT, self->buf, 3));
		//self->up = ((((long)self->buf[0] <<16) | ((long)self->buf[1] <<8) | ((long)self->buf[2])) >> (8-BMP085_MODE)); 
		self->up = self->up >> (8-BMP085_MODE); 
		
		static timestamp_t tfps = 0; 
		static int fps = 0; 
		if(timestamp_expired(tfps)){
			printf("BMP FPS: %d\n", fps); 
			fps = 0; 
			tfps = timestamp_from_now_us(1000000); 
		} fps++; 
		
		//TIMEOUT(10000); 
		
		PT_YIELD(thr); 
		/*
		{
			int16_t t = 10000 - ((2 + (3<<BMP085_MODE)) * 1000L) - 5000; 
			if(t > 0) {
				TIMEOUT(t); 
			}
			else PT_YIELD(thr); 
		}*/
	}
	
	PT_END(thr); 
}

void bmp085_init(struct bmp085 *self, block_dev_t i2c_dev) {
	//i2cblk_init(&self->i2cblk, i2c, addr, I2CBLK_IADDR8); 
	//self->dev = i2cblk_get_interface(&self->i2cblk); 
	self->dev = i2c_dev; 
	self->ut = self->up = 0; 
	
	// one thread per bmp sensor
	libk_create_thread(&self->thread, _bmp085_thread, "bmp085"); 
}

