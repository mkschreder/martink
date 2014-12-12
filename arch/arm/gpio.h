#pragma once

/*
typedef enum {
	GPIO_NONE = 0,
	
} gpio_pin_t;


typedef enum {
	GPNONE = 0,
} aux_pin_t;
*/

extern void 		gpio_write_pin(gpio_pin_t pin, uint8_t value);
extern uint8_t 	gpio_read_pin(gpio_pin_t pin);
extern void 		gpio_set_function(gpio_pin_t pin, uint8_t fun);
extern void 		gpio_set_pullup(gpio_pin_t pin, uint8_t pullup);
