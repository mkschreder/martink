/*
	RTC Device Interface

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

#pragma once

#include <inttypes.h>
#include <i2c/i2c.h>

#define RTC_FIELD_HOUR _BV(1)
#define RTC_FIELD_MINUTE _BV(2)
#define RTC_FIELD_SECOND _BV(3)
#define RTC_FIELD_YEAR _BV(4)
#define RTC_FIELD_MONTH _BV(5)
#define RTC_FIELD_DATE _BV(6)
#define RTC_FIELD_DAY _BV(6)

typedef uint32_t rtc_field_flags_t; 

struct rtc_date_time {
	uint8_t hour, minute, second; 
	uint8_t year, month, date, day; 
}; 

struct rtc_device; 
struct rtc_device_ops {
	int (*get_datetime)(struct rtc_device *self, struct rtc_date_time *time, rtc_field_flags_t flags); 
	int (*set_datetime)(struct rtc_device *self, struct rtc_date_time *time, rtc_field_flags_t flags); 
	int (*get_alarm)(struct rtc_device *self, uint8_t id, struct rtc_date_time *time, rtc_field_flags_t flags); 
	int (*set_alarm)(struct rtc_device *self, uint8_t id, struct rtc_date_time *time, rtc_field_flags_t flags); 
}; 

struct rtc_device {
	struct rtc_device_ops *ops; 
}; 

#define rtc_get_datetime(dev, time, flags) (dev)->ops->get_datetime(dev, time, flags)

struct rtc_device *ds3231_new(struct i2c_adapter *adapter, uint8_t addr); 
