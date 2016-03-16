/*
	ds3231 rtc driver

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

#include "rtc.h"

#define DS3231_DEFAULT_ADDRESS              0x68            

#define DS3231_REG_SECONDS 			0x00
#define DS3231_REG_MINUTES			0x01
#define DS3231_REG_HOURS			0x02
#define DS3231_REG_DAY				0x03 
#define DS3231_REG_DATE				0x04
#define DS3231_REG_MONTH			0x05                            
#define DS3231_REG_YEAR				0x06                    
#define DS3231_REG_ALRM1_SECONDS	0x07       
#define DS3231_REG_ALRM1_MINUTES	0x08 
#define DS3231_REG_ALRM1_HOURS		0x09           
#define DS3231_REG_ALRM1_DATE		0x0A  
#define DS3231_REG_ALRM2_MINUTES	0x0B   
#define DS3231_REG_ALRM2_HOURS		0x0C 
#define DS3231_REG_ALRM2_DATE		0x0D
#define DS3231_REG_CONTROL			0x0E
#define DS3231_REG_STATUS			0x0F 
#define DS3231_REG_AGEOFFSET		0x10
#define DS3231_REG_TEMPH			0x11
#define DS3231_REG_TEMPL			0x12 
                                 

#define DS3231_CLOCK_24H			0
#define DS3231_CLOCK_12H			1 
#define DS3231_AM					0
#define DS3231_PM					1

#include <arch/soc.h>
#include <kernel/mt.h>
#include <kernel/types.h>
#include <kernel/errno.h>
#include <i2c/i2c.h>

struct ds3231 {
	struct rtc_device device; 
	struct i2c_adapter *adapter; 
	uint8_t addr; 
}; 

static unsigned char bcd_to_decimal(unsigned char d) {                                                                                          
	return ((d & 0x0F) + (((d & 0xF0) >> 4) * 10));
}                                

static unsigned char decimal_to_bcd(unsigned char d){
	return (((d / 10) << 4) & 0xF0) | ((d % 10) & 0x0F);
}                                                    

static int ds3231_read_reg(struct ds3231 *self, uint8_t reg, uint8_t *out){                     
	if(i2c_transfer(self->adapter, self->addr, (char*)&reg, 1, (char*)out, 1) < 0) return -EIO; 	
	return 0; 
}                     

static int ds3231_write_reg(struct ds3231 *self, char reg, char value){ 
	char data[] = { reg, value }; 
	if(i2c_write(self->adapter, self->addr, data, 2) < 0) return -EIO; 
	return 0; 
}  

static void ds3231_init(struct ds3231 *self){                                   
	ds3231_write_reg(self, DS3231_REG_CONTROL, 0); 
	ds3231_write_reg(self, DS3231_REG_STATUS, 0); 
}                       

static int ds3231_read(struct rtc_device *dev, struct rtc_date_time *time, rtc_field_flags_t flags){
	struct ds3231 *self = container_of(dev, struct ds3231, device); 
	uint8_t seconds, minutes, hours, year, month, day, date; 
	int32_t ret = 0; 	
	if(!flags) flags = ~flags; 
	if(flags & RTC_FIELD_SECOND) {
		ret -= ds3231_read_reg(self, DS3231_REG_SECONDS, &seconds);  
		time->second = bcd_to_decimal(seconds); 
	} 
	if(flags & RTC_FIELD_MINUTE) {
		ret -= ds3231_read_reg(self, DS3231_REG_MINUTES, &minutes);  
		time->minute = bcd_to_decimal(minutes); 
	} 
	if(flags & RTC_FIELD_HOUR) { 
		ret -= ds3231_read_reg(self, DS3231_REG_HOURS, &hours);  
		time->hour = bcd_to_decimal(hours & 0x3f); 
	} 
	if(flags & RTC_FIELD_YEAR) {
		ret -= ds3231_read_reg(self, DS3231_REG_YEAR, &year); 
		time->year = bcd_to_decimal(year); 
	} 
	if(flags & RTC_FIELD_MONTH) {
		ret -= ds3231_read_reg(self, DS3231_REG_MONTH, &month); 
		time->month = bcd_to_decimal(0x1f & month); 
	} 
	if(flags & RTC_FIELD_DATE) {
		ret -= ds3231_read_reg(self, DS3231_REG_DATE, &date); 
		time->date = bcd_to_decimal(0x3f & date); 
	}
	if(flags & RTC_FIELD_DAY) {
		ret -= ds3231_read_reg(self, DS3231_REG_DAY, &day); 
		time->day = bcd_to_decimal(0x7 & day); 
	}
	// TODO: refactor error return code so it does not write anything if there was an error
	if(ret < 0) return -EIO; 
	return 0; 
}                                  

static int ds3231_write(struct rtc_device *dev, struct rtc_date_time *time, rtc_field_flags_t flags){ 
	struct ds3231 *self = container_of(dev, struct ds3231, device); 
	int ret = 0; 
	if(!flags) flags = ~flags; 
	if(flags & RTC_FIELD_SECOND) ret -= ds3231_write_reg(self, DS3231_REG_SECONDS, decimal_to_bcd(time->second)); 
	if(flags & RTC_FIELD_MINUTE) ret -= ds3231_write_reg(self, DS3231_REG_MINUTES, decimal_to_bcd(time->minute)); 
	if(flags & RTC_FIELD_HOUR) ret -= ds3231_write_reg(self, DS3231_REG_HOURS, 0x3f & decimal_to_bcd(time->hour)); 
	if(flags & RTC_FIELD_DAY) ret -= ds3231_write_reg(self, DS3231_REG_DAY, decimal_to_bcd(time->day)); 
	if(flags & RTC_FIELD_DATE) ret -= ds3231_write_reg(self, DS3231_REG_DATE, decimal_to_bcd(time->date)); 
	if(flags & RTC_FIELD_MONTH) ret -= ds3231_write_reg(self, DS3231_REG_MONTH, decimal_to_bcd(time->month)); 
	if(flags & RTC_FIELD_YEAR) ret -= ds3231_write_reg(self, DS3231_REG_YEAR, decimal_to_bcd(time->year)); 
	if(ret < 0) return -EIO; 
	return 0; 
}

static int ds3231_set_alarm1(struct ds3231 *self, struct rtc_date_time *time, rtc_field_flags_t flags){
	int ret = 0; 
	if(!flags) flags = ~flags; 
	if(flags & RTC_FIELD_SECOND) ret -= ds3231_write_reg(self, DS3231_REG_ALRM1_SECONDS, 0x7f & decimal_to_bcd(time->second)); 
	if(flags & RTC_FIELD_MINUTE) ret -= ds3231_write_reg(self, DS3231_REG_ALRM1_MINUTES, 0x7f & decimal_to_bcd(time->minute)); 
	if(flags & RTC_FIELD_HOUR) ret -= ds3231_write_reg(self, DS3231_REG_ALRM1_HOURS, 0x3f & decimal_to_bcd(time->hour)); 
	if(ret < 0) return -EIO; 
	return 0; 
}

static int ds3231_set_alarm2(struct ds3231 *self, struct rtc_date_time *time, rtc_field_flags_t flags){
	int ret = 0; 
	if(!flags) flags = ~flags; 
	//if(flags & RTC_FIELD_SECOND) ret -= ds3231_write_reg(self, DS3231_REG_ALRM2_SECONDS, 0x7f & decimal_to_bcd(time->second)); 
	if(flags & RTC_FIELD_MINUTE) ret -= ds3231_write_reg(self, DS3231_REG_ALRM2_MINUTES, 0x7f & decimal_to_bcd(time->minute)); 
	if(flags & RTC_FIELD_HOUR) ret -= ds3231_write_reg(self, DS3231_REG_ALRM2_HOURS, 0x3f & decimal_to_bcd(time->hour)); 
	if(ret < 0) return -EIO; 
	return 0; 
}

static int ds3231_get_alarm1(struct ds3231 *self, struct rtc_date_time *time, rtc_field_flags_t flags){
	int ret = 0; 
	uint8_t hours, minutes, seconds; 
	if(!flags) flags = ~flags; 
	if(flags & RTC_FIELD_SECOND) {
		ret -= ds3231_read_reg(self, DS3231_REG_ALRM1_SECONDS, &seconds); 
		seconds = bcd_to_decimal(seconds);
	} 
	if(flags & RTC_FIELD_MINUTE) {
		ret -= ds3231_read_reg(self, DS3231_REG_ALRM1_MINUTES, &minutes); 
		minutes = bcd_to_decimal(minutes);   
	}
	if(flags & RTC_FIELD_HOUR){	
		ret -= ds3231_read_reg(self, DS3231_REG_ALRM1_HOURS, &hours); 
		hours = bcd_to_decimal(hours);   
	}
	if(ret < 0) return -EIO; 
	time->second = seconds; 
	time->minute = minutes; 
	time->hour = hours; 
	return 0; 
}

static int ds3231_get_alarm2(struct ds3231 *self, struct rtc_date_time *time, rtc_field_flags_t flags){
	int ret = 0; 
	uint8_t hours, minutes, seconds; 
	//ret -= ds3231_read_reg(self, DS3231_REG_ALRM2_SECONDS, &seconds); 
	ret -= ds3231_read_reg(self, DS3231_REG_ALRM2_MINUTES, &minutes); 
	ret -= ds3231_read_reg(self, DS3231_REG_ALRM2_HOURS, &hours); 
	if(ret < 0) return -EIO; 
	time->hour = bcd_to_decimal(hours);   
	time->minute = bcd_to_decimal(minutes);   
	//time->second = bcd_to_decimal(seconds);   
	return 0; 
}

#if 0
static int ds3231_read_temp(struct ds3231 *self, uint16_t *temp){
	uint8_t th, tl; 
	int ret = 0; 
	ret -= ds3231_read_reg(self, DS3231_REG_TEMPH, &th); 
	ret -= ds3231_read_reg(self, DS3231_REG_TEMPL, &tl); 
	if(ret < 0) return -EIO; 
	*temp = ((uint16_t)th << 8) | tl; 
	return 0; 
}
#endif

static int ds3231_set_alarm(struct rtc_device *dev, uint8_t id, struct rtc_date_time *time, rtc_field_flags_t flags){
	struct ds3231 *self = container_of(dev, struct ds3231, device); 
	switch(id) {
		case 0: return ds3231_set_alarm1(self, time, flags); 
		case 1: return ds3231_set_alarm2(self, time, flags); 
		default: return -EINVAL; 
	}
}

static int ds3231_get_alarm(struct rtc_device *dev, uint8_t id, struct rtc_date_time *time, rtc_field_flags_t flags){
	struct ds3231 *self = container_of(dev, struct ds3231, device); 
	switch(id) {
		case 0: return ds3231_get_alarm1(self, time, flags); 
		case 1: return ds3231_get_alarm2(self, time, flags); 
		default: return -EINVAL; 
	}
}

struct rtc_device_ops ds3231_ops = {
	.get_datetime = ds3231_read, 
	.set_datetime = ds3231_write, 
	.get_alarm = ds3231_get_alarm, 
	.set_alarm = ds3231_set_alarm
}; 

struct rtc_device *ds3231_new(struct i2c_adapter *adapter, uint8_t addr){
	struct ds3231 *self = kzmalloc(sizeof(struct ds3231)); 
	if(!self) return NULL; 
	self->adapter = adapter; 
	self->addr = addr; 
	self->device.ops = &ds3231_ops; 
	ds3231_init(self); 
	return &self->device; 
}

#if 0
static int ds3231_read_temp(struct ds3231 *self, float *temp){
	uint8_t th, tl; 
	int ret = 0; 
	ret -= ds3231_read_reg(self, DS3231_REG_TEMPH, &th); 
	ret -= ds3231_read_reg(self, DS3231_REG_TEMPL, &tl); 
	if(ret < 0) return -EIO; 
	tl >>= 6; 
	*temp ((float)tl * 0x25 + th)
	return 0; 
}
#endif
