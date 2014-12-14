/**
	This is a fast GPIO implementation for atmega

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

#include "../../gpio.h"

enum {
	GPIO_NONE = 0,
	GPIO_PA0_NC,
	GPIO_PA1_NC,
	GPIO_PA2_NC,
	GPIO_PA3_NC,
	GPIO_PA4_NC,
	GPIO_PA5_NC,
	GPIO_PA6_NC,
	GPIO_PA7_NC,
	GPIO_PB0,
	GPIO_PB1,
	GPIO_PB2,
	GPIO_PB3,
	GPIO_PB4,
	GPIO_PB5,
	GPIO_PB6,
	GPIO_PB7,
	GPIO_PC0,
	GPIO_PC1,
	GPIO_PC2,
	GPIO_PC3,
	GPIO_PC4,
	GPIO_PC5,
	GPIO_PC6,
	GPIO_PC7_NC,
	GPIO_PD0,
	GPIO_PD1,
	GPIO_PD2,
	GPIO_PD3,
	GPIO_PD4,
	GPIO_PD5,
	GPIO_PD6,
	GPIO_PD7,
};

#define GPIO_ICP1 	GPIO_PB0
#define GPIO_CLK0 	GPIO_PB0
#define GPIO_OC1A 	GPIO_PB1
#define GPIO_OC1B 	GPIO_PB2
#define GPIO_SS			GPIO_PB2
#define GPIO_OC2A 	GPIO_PB3
#define GPIO_MOSI 	GPIO_PB3
#define GPIO_MISO 	GPIO_PB4
#define GPIO_SCK		GPIO_PB5
#define GPIO_TOSC1 	GPIO_PB6
#define GPIO_TOSC2 	GPIO_PB7
#define GPIO_ADC0 	GPIO_PC0
#define GPIO_ADC1 	GPIO_PC1
#define GPIO_ADC2 	GPIO_PC2
#define GPIO_ADC3 	GPIO_PC3
#define GPIO_ADC4 	GPIO_PC4
#define GPIO_SDA 		GPIO_PC4
#define GPIO_ADC5 	GPIO_PC5
#define GPIO_SCL 		GPIO_PC5
#define GPIO_RESET 	GPIO_PC6
#define GPIO_RXD 		GPIO_PD0
#define GPIO_TXD 		GPIO_PD1
#define GPIO_INT0 	GPIO_PD2
#define GPIO_INT1 	GPIO_PD3
#define GPIO_XCK 		GPIO_PD4
#define GPIO_T0 		GPIO_PD4
#define GPIO_OC0B 	GPIO_PD5
#define GPIO_T1 		GPIO_PD5
#define GPIO_OC0A 	GPIO_PD6
#define GPIO_AIN0 	GPIO_PD6
#define GPIO_AIN1 	GPIO_PD7

extern const struct pin_decl {
	volatile uint8_t *out_reg;
	volatile uint8_t *in_reg;
	volatile uint8_t	*ddr_reg;
} gPinPorts[4];


#define REG_IDX(pin) 	((pin > 0)?(((pin - 1) & 0xf8) >> 3):0)
#define PIN_IDX(pin) 	((pin > 0)?((pin - 1) & 0x07):0)
#define OUT_REG(pin) 	*(gPinPorts[REG_IDX(pin)].out_reg)
#define IN_REG(pin) 	*(gPinPorts[REG_IDX(pin)].in_reg)
#define DDR_REG(pin) 	*(gPinPorts[REG_IDX(pin)].ddr_reg)

#define gpio_set_function(pin, fun) (\
	(fun == GP_OUTPUT)\
		?(DDR_REG(pin) |= _BV(PIN_IDX(pin)))\
		:(DDR_REG(pin) &= ~_BV(PIN_IDX(pin)))\
)

#define gpio_set_pullup(pin, pull) (\
	gpio_set_function(pin, GP_INPUT),\
	(pull)\
		?(OUT_REG(pin) |= _BV(PIN_IDX(pin)))\
		:(OUT_REG(pin) &= ~_BV(PIN_IDX(pin)))\
)

#define gpio_write_pin(pin, val) (\
	(val)\
		?(OUT_REG(pin) |= _BV(PIN_IDX(pin)))\
		:(OUT_REG(pin) &= ~_BV(PIN_IDX(pin)))\
)

#define gpio_read_pin(pin) (\
	(IN_REG(pin) & PIN_IDX(pin))?1:0\
)

#define gpio_clear(pin) gpio_write_pin(pin, 0)
#define gpio_set(pin) gpio_write_pin(pin, 1)
