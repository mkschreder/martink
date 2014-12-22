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

#include "interface.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MINCOMMAND 800

#define PWM_MIN MINCOMMAND
#define PWM_MAX 2000

enum {
	RC_THROTTLE = 0, 
	RC_YAW 			= 1, 
	RC_PITCH 		= 2,
	RC_ROLL 		= 3,
	RC_MODE 		= 4,
	RC_MODE2		= 5
};

void due_init(void); 
void due_process_events(void);
struct fc_quad_interface due_get_fc_quad_interface(void);

#define DUE_LED_PIN GPIO_PB27

// flight controller interface
#define fc_init() due_init()
#define fc_process_events() due_process_events()
#define fc_get_interface() due_get_fc_quad_interface()
#define FC_LED_PIN DUE_LED_PIN

#ifdef __cplusplus
}
#endif
