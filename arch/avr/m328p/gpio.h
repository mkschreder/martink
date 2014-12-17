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

#include "../../types.h"

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
	GPIO_COUNT
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
#define GPIO_OC2B 	GPIO_PD3
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
	volatile uint8_t *ddr_reg;
} gPinPorts[4];

struct pin_state {
	volatile timestamp_t t_up; 
	volatile timestamp_t t_down; 
	volatile uint8_t status; 
}; 

extern volatile struct pin_state gPinState[GPIO_COUNT - GPIO_PB0]; 

#define RIDX(pin) 	((pin > 0)?(((pin - 1) & 0xf8) >> 3):0)
#define PIDX(pin) 	((pin > 0)?((pin - 1) & 0x07):0)
#define OREG(pin) 	*(gPinPorts[RIDX(pin)].out_reg)
#define IREG(pin) 	*(gPinPorts[RIDX(pin)].in_reg)
#define DREG(pin) 	*(gPinPorts[RIDX(pin)].ddr_reg)
#define RSET(reg, bit) (reg |= _BV(bit))
#define RCLR(reg, bit) (reg &= ~_BV(bit))

#define gpio_configure(pin, fun) (\
	((fun) & GP_OUTPUT)\
		?RSET(DREG(pin), PIDX(pin))\
		:RCLR(DREG(pin), PIDX(pin)), \
	(((fun) & GP_PULL) && ((fun) & GP_PULLUP))\
		?RSET(OREG(pin), PIDX(pin))\
		:RCLR(OREG(pin), PIDX(pin)), \
	((fun) & GP_PCINT)?gpio_enable_pcint(pin):(0)\
)

static inline uint8_t gpio_get_status(gpio_pin_t pin, 
	timestamp_t *ch_up, timestamp_t *ch_down){
	pin = (pin < GPIO_PB0)?GPIO_PB0:((pin > GPIO_PD7)?GPIO_PD7:pin); 
	uint8_t ret = 0;
	volatile struct pin_state *st = &gPinState[pin - GPIO_PB0]; 
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ret = st->status; 
		*ch_up = st->t_up; 
		*ch_down = st->t_down; 
		st->status = 0; 
	}
	return ret; 
}

#define gpio_write_word(addr, value) (\
	((addr) >= 0 && (addr) < 4)?(*gPinPorts[addr].out_reg = ((value) & 0xff), 0):(1)\
)

#define gpio_read_word(addr, value) (\
	((addr) >= 0 && (addr) < 4)?(*value = *gPinPorts[addr].in_reg, 0):(1)\
)

#define gpio_write_pin(pin, val) (\
	(val)?RSET(OREG(pin), PIDX(pin)):RCLR(OREG(pin), PIDX(pin))\
)

#define gpio_read_pin(pin) (\
	(IREG(pin) & PIDX(pin))?1:0 \
)

#define gpio_clear(pin) gpio_write_pin(pin, 0)
#define gpio_set(pin) gpio_write_pin(pin, 1)

// evaluates to bit that has been enabled or -1 if invalid pcint
#define gpio_enable_pcint(pin) (\
	((pin) >= GPIO_PB0 && (pin) <= GPIO_PB7)\
		?(PCICR |= _BV(PCINT0), PCMSK0 = PCMSK0 | _BV((pin) - GPIO_PB0))\
		:((pin) >= GPIO_PC0 && (pin) <= GPIO_PC7_NC)\
			?(PCICR |= _BV(PCINT1), PCMSK1 = PCMSK1 | _BV((pin) - GPIO_PB0))\
			:((pin) >= GPIO_PD0 && (pin) <= GPIO_PD7)\
				?(PCICR |= _BV(PCINT2), PCMSK2 = PCMSK2 | _BV((pin) - GPIO_PD0))\
				:(-1)\
)

#define gpio_disable_pcint(pin) (\
	((pin) >= GPIO_PB0 && (pin) <= GPIO_PB7)\
		?(PCICR &= ~_BV(PCINT0), PCMSK0 = PCMSK0 & ~_BV((pin) - GPIO_PB0))\
		:((pin) >= GPIO_PC0 && (pin) <= GPIO_PC7)\
			?(PCICR &= ~_BV(PCINT1), PCMSK1 = PCMSK1 & ~ _BV((pin) - GPIO_PB0))\
			:((pin) >= GPIO_PD0 && (pin) <= GPIO_PD7)\
				?(PCICR &= ~_BV(PCINT2), PCMSK2 = PCMSK2 & ~ _BV((pin) - GPIO_PD0))\
				:(-1)\
)
