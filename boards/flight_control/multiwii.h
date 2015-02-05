/*
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

/** IMPORTANT: 
 Note that on Multiwii, the magnetometer is connected to aux port 
 of the mpu6050 device so in order to access it over i2c, one has 
 to enable pass through on the mpu6050 first. In libk mpu6050 driver
 this is done automatically. 
*/

#include <inttypes.h>
#include "../interface.h"

/*
#define PWM_MIN MINCOMMAND
#define PWM_MAX MAXCOMMAND
*/

void mwii_init(void); 
void mwii_process_events(void);

fc_board_t mwii_get_fc_quad_interface(void);

void mwii_calibrate_escs(void); 

#define MWII_LED_PIN GPIO_PB5

enum {
	RC_THROTTLE = 0, 
	RC_YAW 			= 1, 
	RC_PITCH 		= 2,
	RC_ROLL 		= 3,
	RC_MODE 		= 4,
	RC_MODE2		= 5
};

#define fc_init() mwii_init()
#define fc_process_events() mwii_process_events()
#define fc_interface() mwii_get_fc_quad_interface()
#define fc_led_on() gpio_set(MWII_LED_PIN)
#define fc_led_off() gpio_clear(MWII_LED_PIN)
#define fc_calibrate_escs() mwii_calibrate_escs()

#ifdef __cplusplus
}
#endif
