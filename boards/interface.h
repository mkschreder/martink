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

struct fc_quad_rx_input {
	uint16_t 		channel[6];
};

struct fc_quad_interface {
	void 				(*read_accelerometer)(struct fc_quad_interface *self,
		float *ax, float *ay, float *az); 
	void 				(*read_gyroscope)(struct fc_quad_interface *self,
		float *gx, float *gy, float *gz);
	void 				(*read_magnetometer)(struct fc_quad_interface *self,
		float *mx, float *my, float *mz);
	int16_t			(*read_altitude)(struct fc_quad_interface *self);
	int16_t			(*read_pressure)(struct fc_quad_interface *self);
	int16_t 		(*read_temperature)(struct fc_quad_interface *self);

	void 				(*read_receiver)(struct fc_quad_interface *self,
		uint16_t *rc_thr, uint16_t *rc_pitch, uint16_t *rc_yaw, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1); 
	void				(*write_motors)(struct fc_quad_interface *self,
		uint16_t front, uint16_t back, uint16_t left, uint16_t right);
}; 
