/*
	CC3D flight controller board driver.

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

/*
=== Current status === 
	Uart1:			yes
	Uart3: 			no
	USB:				no
	I2C:				no
	Gyro: 			yes
	RC Input: 	yes, except chan 3 and 4 (tim3_3 and tim3_4, pb0, pb1) are not working. 
	PWM Out: 		yes
	Flash: 			yes
*/
#pragma once

#include <arch/soc.h>

#include "../interface.h"

#define MINCOMMAND 1000
#define MAXCOMMAND 1700

#define GPIO_RC_CH1			GPIO_PB6
#define GPIO_RC_CH2			GPIO_PB5
#define GPIO_RC_CH3			GPIO_PB0
#define GPIO_RC_CH4			GPIO_PB1
#define GPIO_RC_CH5 		GPIO_PA0
#define GPIO_RC_CH6 		GPIO_PA1
#define GPIO_PWM1				GPIO_PB9
#define GPIO_PWM2				GPIO_PB8
#define GPIO_PWM3				GPIO_PB7
#define GPIO_PWM4				GPIO_PA8
#define GPIO_PWM5				GPIO_PB4
#define GPIO_PWM6				GPIO_PA2
#define GPIO_GYRO_INT		GPIO_PA3
#define GPIO_GYRO_CS		GPIO_PA4
#define GPIO_GYRO_SCK		GPIO_PA5
#define GPIO_GYRO_MIS		GPIO_PA6
#define GPIO_GYRO_MOS 	GPIO_PA7
#define GPIO_UART1_TX		GPIO_PA9
#define GPIO_UART1_RX 	GPIO_PA10
#define GPIO_UART1_XOR 	GPIO_PB2
#define GPIO_USB_DM			GPIO_PA11
#define GPIO_USB_DP			GPIO_PA12
#define GPIO_STATUS_LED	GPIO_PB3
#define GPIO_FLASH_CS 	GPIO_PB12

#define CC3D_LED_PIN GPIO_PB3

#ifdef __cplusplus
extern "C" {
#endif

/*
#define FC_PWM_CH1 PWM_CH44
#define FC_PWM_CH2 PWM_CH43
#define FC_PWM_CH3 PWM_CH42
#define FC_PWM_CH4 PWM_CH11
#define FC_PWM_CH5 PWM_CH31
#define FC_PWM_CH6 PWM_CH23

#define FC_PWM_RC1 PWM_CH41
#define FC_PWM_RC2 PWM_CH32
#define FC_PWM_RC3 PWM_CH33
#define FC_PWM_RC4 PWM_CH34
#define FC_PWM_RC5 PWM_CH21
#define FC_PWM_RC6 PWM_CH22
*/
typedef enum {
	CC3D_IN_PWM1 = PWM_CH41, 
	CC3D_IN_PWM2 = PWM_CH32, 
	CC3D_IN_PWM3 = PWM_CH33, 
	CC3D_IN_PWM4 = PWM_CH34, 
	CC3D_IN_PWM5 = PWM_CH21, 
	CC3D_IN_PWM6 = PWM_CH22
} cc3d_input_pwm_id_t; 

typedef enum {
	CC3D_OUT_PWM1 = PWM_CH44, 
	CC3D_OUT_PWM2 = PWM_CH43, 
	CC3D_OUT_PWM3 = PWM_CH42, 
	CC3D_OUT_PWM4 = PWM_CH11, 
	CC3D_OUT_PWM5 = PWM_CH31, 
	CC3D_OUT_PWM6 = PWM_CH23
} cc3d_output_pwm_id_t; 

/// initializes the board and all peripherals
void cc3d_init(void); 

/// process events must be called every frame
void cc3d_process_events(void);

void cc3d_read_receiver(
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1); 
void cc3d_write_motors(uint16_t front, uint16_t back, uint16_t left, uint16_t right); 
/// reads all sensors
void cc3d_read_sensors(struct fc_data *data); 

//*************************
// CONFIG
//*************************

/// reads config from internal flash
int8_t cc3d_read_config(uint8_t *data, uint16_t size);

/// writes config to internal flash
int8_t cc3d_write_config(const uint8_t *data, uint16_t size);

typedef enum {
	CC3D_FLEXIPORT_UART, 
	CC3D_FLEXIPORT_I2C
}cc3d_flexi_port_mode_t; 

/// configures flexiport mode
void cc3d_configure_flexiport(cc3d_flexi_port_mode_t mode); 

//*************************
// INPUTS and OUTPUTS
//*************************

/// reads input pwm channels
uint16_t cc3d_read_pwm(cc3d_input_pwm_id_t chan); 

/// writes output pwm to motor outputs
void cc3d_write_pwm(cc3d_output_pwm_id_t chan, uint16_t value);

//*************************
// SENSORS
//*************************

/// reads last measured acceleration in G
void cc3d_read_acceleration_g(float *ax, float *ay, float *az); 
/// reads last measured angular velocity in degrees / sec
void cc3d_read_angular_velocity_dps(float *gyrx, float *gyry, float *gyrz); 

//*************************
// STATUS
//*************************

#define cc3d_led_on() gpio_clear(GPIO_PB3)
#define cc3d_led_off() gpio_set(GPIO_PB3)

//*************************
// INTERFACES
//*************************
/// get generic serial interface for mainport

serial_dev_t cc3d_get_mainport_serial_interface(void); 

/// get generic serial interface for flexiport
/// flexiport must be configured in uart mode
serial_dev_t cc3d_get_flexiport_serial_interface(void); 

/// get generic i2c interface for flexiport
i2c_dev_t cc3d_get_flexiport_i2c_interface(void); 

/// get generic flight controller interface
fc_board_t cc3d_get_fc_quad_interface(void);

#ifdef __cplusplus
}
#endif

/*
#define fc_init() cc3d_init()
#define fc_process_events() cc3d_process_events()
#define fc_interface() cc3d_get_fc_quad_interface()
#define fc_led_on() gpio_clear(GPIO_PB3)
#define fc_led_off() gpio_set(GPIO_PB3)
*/
