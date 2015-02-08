/**
	This file is part of martink project.

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

#pragma once

#include "flight_control/interface.h"
#include "../arch/interface.h"

struct fc_quad_rx_input {
	uint16_t 		channel[6];
};

struct fc_quad_interface; 
typedef struct fc_quad_interface **fc_board_t; 

typedef enum {
	HAVE_ACC = (1 << 0), 
	HAVE_GYR = (1 << 1),
	HAVE_MAG = (1 << 2),
	HAVE_BAR = (1 << 3),
	HAVE_BATTERY_MONITOR = (1 << 4),
	HAVE_TEMP = (1 << 1),
} fc_flags_t; 

struct fc_data {
	fc_flags_t flags; 
	struct {
		float x, y, z; 
	} acc_g; 
	struct {
		float x, y, z; 
	} gyr_deg; 
	struct {
		float x, y, z; 
	} mag; 
	struct {
		int16_t x, y, z; 
	} raw_acc; 
	struct {
		int16_t x, y, z; 
	} raw_gyr; 
	struct {
		int16_t x, y, z; 
	} raw_mag; 
	// altitude above sealevel based on pressure data 
	float atmospheric_altitude; 
	// altitude in meters based on echo from sonar (-1 if sonar not available)
	float sonar_altitude; 
	float pressure, temperature; 
	float vbat; 
}; 

struct fc_quad_interface {
	int8_t 		(*read_sensors)(fc_board_t self, struct fc_data *data); 
	int8_t 		(*read_receiver)(fc_board_t self,
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1); 
	int8_t				(*write_motors)(fc_board_t self,
		uint16_t front, uint16_t back, uint16_t left, uint16_t right);
		
	// return -1 on fail and 0 on success
	int8_t (*write_config)(fc_board_t self, const uint8_t *data, uint16_t size);
	// return -1 on fail and 0 on success
	int8_t (*read_config)(fc_board_t self, uint8_t *data, uint16_t size);
	
	/// should return a serial interface to the communication channel with PC
	serial_dev_t (*get_pc_link_interface)(fc_board_t self); 
	/// 
	///adc_dev_t (*get_battery_monitor)
}; 

/*
#define fc_read_accelerometer(self, ax, ay, az) (*self)->read_accelerometer(self, ax, ay, az)
#define fc_read_gyroscope(self, gx, gy, gz) (*self)->read_gyroscope(self, gx, gy, gz)
#define fc_read_magnetometer(self, mx, my, mz) (*self)->read_magnetometer(self, mx, my, mz)
#define fc_read_altitude(self) (*self)->read_altitude(self)
#define fc_read_pressure(self) (*self)->read_pressure(self)
#define fc_read_temperature(self) (*self)->read_temperature(self)
#define fc_read_battery_monitor(self) (*self)->read_battery_monitor(self)
*/
#define fc_read_sensors(self, sensors) (*self)->read_sensors(self, sensors)
#define fc_read_receiver(self, thr, yaw, pitch, roll, aux0, aux1) \
	(*self)->read_receiver(self, thr, yaw, pitch, roll, aux0, aux1)
#define fc_write_motors(self, front, back, left, right) \
	(*self)->write_motors(self, front, back, left, right)
#define fc_write_config(self, data, size) \
	(*self)->write_config(self, data, size)
#define fc_read_config(self, data, size) \
	(*self)->read_config(self, data, size)
#define fc_get_pc_link_interface(self) (*self)->get_pc_link_interface(self)
