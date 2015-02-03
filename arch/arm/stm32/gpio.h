#pragma once

#include <arch/types.h>

enum {
	GPIO_PA0,
	GPIO_PA1,
	GPIO_PA2,
	GPIO_PA3,
	GPIO_PA4,
	GPIO_PA5,
	GPIO_PA6,
	GPIO_PA7,
	GPIO_PA8,
	GPIO_PA9,
	GPIO_PA10,
	GPIO_PA11,
	GPIO_PA12,
	GPIO_PA13,
	GPIO_PA14,
	GPIO_PA15,
	GPIO_PB0,
	GPIO_PB1,
	GPIO_PB2,
	GPIO_PB3,
	GPIO_PB4,
	GPIO_PB5,
	GPIO_PB6,
	GPIO_PB7,
	GPIO_PB8,
	GPIO_PB9,
	GPIO_PB10,
	GPIO_PB11,
	GPIO_PB12,
	GPIO_PB13,
	GPIO_PB14,
	GPIO_PB15,
	GPIO_COUNT
}; 

void gpio_init_default(void); 

void gpio_configure(gpio_pin_t pin, uint16_t flags); 

#define gpio_write_word(addr, value) (0)
#define gpio_read_word(addr, value) (0)
void gpio_write_pin(gpio_pin_t pin, uint8_t val); 
uint8_t gpio_read_pin(gpio_pin_t pin); 

uint16_t gpio_get_status(gpio_pin_t pin, timestamp_t *t_up, timestamp_t *t_down); 
