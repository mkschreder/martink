/**
	Fast macro based SPI interface for AVR Mega 328P

	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

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
	Github: https://github.com/mkschreder
*/

#pragma once

// GPIO pin functions
enum {
	/// configures a gpio pin as input
	GP_INPUT 		= (1 << 0),
	/// configures a gpio pin as output
	GP_OUTPUT 	= (1 << 1),
	/// enables pull(up/down) on gpio pin
	//GP_PULL 		= (1 << 2),
	/// no pullup on gpio pin
	GP_HIZ			= (1 << 3),
	/// enable pullup
	GP_PULLUP 	= (1 << 4),
	/// enable pulldown (if arch supports it)
	GP_PULLDOWN = (1 << 5),
	/// alternative function on the pin
	GP_AF 			= (1 << 6),
	/// output open drain mode
	GP_OPEN_DRAIN		= (1 << 7),
	/// push pull output
	GP_PUSH_PULL 	= (1 << 8), 
	/// analog input
	GP_ANALOG 	= (1 << 9)
}; 

/// pin change interrupt will run when pin gets toggled
/// make sure that pin change handler is defined
#define GP_PCINT		(1 << 3)

// GPIO status flags
/// gpio status: pin level has changed
#define GP_CHANGED 		(1 << 0)
/// gpio status: pin went high
#define GP_WENT_HIGH 	(GP_CHANGED | (1 << 1))
/// gpio status: pin went low
#define GP_WENT_LOW 	(GP_CHANGED | (1 << 2))

//void gpio_init(void);

/// returns a standard parallel interface for the hardware gpio pins
/// as a \ref struct parallel_interface
//pio_dev_t gpio_get_parallel_interface(void); 

struct gpio_adapter; 
struct gpio_adapter_ops {
	void (*set_pin)(struct gpio_adapter *self, gpio_pin_t pin); 	
	void (*clear_pin)(struct gpio_adapter *self, gpio_pin_t pin); 	
	void (*configure)(struct gpio_adapter *self, gpio_pin_t pin, uint16_t fun); 
}; 

#define gpio_configure_pin(adapter, pin, opts) (adapter)->ops->configure(adapter, pin, opts)
#define gpio_set_pin(adapter, pin) (adapter)->ops->set_pin(adapter, pin)
#define gpio_clear_pin(adapter, pin) (adapter)->ops->clear_pin(adapter, pin)

struct gpio_adapter {
	struct gpio_adapter_ops *ops; 
}; 

