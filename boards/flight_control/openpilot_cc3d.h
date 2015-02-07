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

#ifdef __cplusplus
extern "C" {
#endif

void cc3d_init(void); 
void cc3d_process_events(void);

fc_board_t cc3d_get_fc_quad_interface(void);

#ifdef __cplusplus
}
#endif

#define fc_init() cc3d_init()
#define fc_process_events() cc3d_process_events()
#define fc_interface() cc3d_get_fc_quad_interface()
#define fc_led_on() gpio_set(0); 
#define fc_led_off() gpio_clear(0); 
