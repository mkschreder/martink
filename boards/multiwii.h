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

#ifdef __cplusplus
extern "C" {
#endif

#ifdef BOARD 
#error "Only one board file can be included in a project!"
#endif

#define BOARD

#include <inttypes.h>

#define MINCOMMAND 800

#define PWM_MIN MINCOMMAND
#define PWM_MAX 2000

void get_accelerometer(float *x, float *y, float *z); 
void get_gyroscope(float *x, float *y, float *z); 
void get_magnetometer(int16_t *x, int16_t *y, int16_t *z); 
void get_altitude(int16_t *alt);
void get_pressure(int16_t *pres);
void get_temperature(int16_t *temp);
uint8_t get_rc_commands(int16_t *throt, int16_t *yaw, int16_t *pitch, int16_t *roll); 

enum {
	RC_IN0 = 0, 
	RC_IN1, 
	RC_IN2, 
	RC_IN3, 
	RC_IN4, 
	RC_IN5, 
	PWM_FRONT, 
	PWM_BACK, 
	PWM_LEFT, 
	PWM_RIGHT, 
	PWM_GIMBAL,
	PWM_COUNT, 
	LED_PIN
}; 

enum {
	RC_THROTTLE = RC_IN0, 
	RC_YAW 			= RC_IN1, 
	RC_PITCH 		= RC_IN2,
	RC_ROLL 		= RC_IN3,
	RC_MODE 		= RC_IN4
}; 

void brd_init(void); 
void brd_process_events(void); 

void set_pin(uint8_t pin, uint16_t value);
uint16_t get_pin(uint8_t pin);

#ifdef __cplusplus
}
#endif
