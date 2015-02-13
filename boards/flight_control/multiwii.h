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

typedef enum {
	MWII_OUT_PWM0, 
	MWII_OUT_PWM1,
	MWII_OUT_PWM2, 
	MWII_OUT_PWM3
} mwii_out_pwm_channel_t; 

typedef enum {
	MWII_IN_PWM0, 
	MWII_IN_PWM1, 
	MWII_IN_PWM2, 
	MWII_IN_PWM3
} mwii_in_pwm_channel_t; 

/// initializes all on board peripherals and interfaces
void mwii_init(void); 
/// writes a pwm pulse to specified pwm channel
void mwii_write_pwm(mwii_out_pwm_channel_t chan, uint16_t value); 
/// reads most recent captured pwm pulse length from channel
uint16_t mwii_read_pwm(mwii_in_pwm_channel_t chan); 
/// gets main i2c interface of the board (it only has one) 
i2c_dev_t mwii_get_i2c_interface(void); 
/// gets main usart interface
serial_dev_t mwii_get_uart_interface(void); 

//**********************
// READING SENSOR DATA
//**********************
/// reads last measured acceleration in G
void mwii_read_acceleration_g(float *ax, float *ay, float *az); 
/// reads last measured angular velocity in degrees / sec
void mwii_read_angular_velocity_dps(float *gyrx, float *gyry, float *gyrz); 
/// reads last measured magnetic field in ? 
void mwii_read_magnetic_field(float *mx, float *my, float *mz); 
/// reads temperature in degrees C
float mwii_read_temperature_c(void); 
/// reads pressure in pascal
float mwii_read_pressure_pa(void); 

//************************
// CONFIG 
//************************
/// read config from internal storage
void mwii_read_config(uint8_t *data, uint8_t size); 
/// write config to internal storage
void mwii_write_config(const uint8_t *data, uint8_t size); 

//************************
// UTILITIES
//************************

/// schedules an esc calibration to be done upon next powerup
void mwii_calibrate_escs_on_reboot(void); 

//************************
// PROCESSING FRAME EVENTS
//************************
/// processes any events. Must be called as part of the main loop. 
void mwii_process_events(void);

fc_board_t mwii_get_fc_quad_interface(void);

void mwii_calibrate_escs(void); 

#define MWII_LED_PIN GPIO_PB5

#define mwii_led_on() gpio_set(MWII_LED_PIN)
#define mwii_led_off() gpio_clear(MWII_LED_PIN)

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
