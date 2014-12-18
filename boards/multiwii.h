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
#include "interface.h"

#define MINCOMMAND 800

#define PWM_MIN MINCOMMAND
#define PWM_MAX 2000

/*
void mwii_read_accelerometer(double *x, double *y, double *z);
void mwii_read_gyroscope(double *x, double *y, double *z);
void mwii_read_magnetometer(int16_t *x, int16_t *y, int16_t *z);
int16_t mwii_read_altitude(void); 
int16_t mwii_read_pressure(void); 
int16_t mwii_read_temperature(void);
*/
void mwii_init(void); 
void mwii_process_events(void);
/*
void mwii_write_motors(uint16_t front, uint16_t back, uint16_t left, uint16_t right);
uint8_t mwii_read_receiver(struct rx_receiver_values *in);
*/
struct fc_quad_interface mwii_get_fc_quad_interface(void);

#define MWII_LED_PIN GPIO_PB5

enum {
	RC_THROTTLE = 0, 
	RC_YAW 			= 1, 
	RC_PITCH 		= 2,
	RC_ROLL 		= 3,
	RC_MODE 		= 4,
	RC_MODE2		= 5
};

/*
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
*/

#ifdef __cplusplus
}
#endif
