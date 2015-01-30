#pragma once

enum {
	GPIO_DUMMY,
	GPIO_COUNT
}; 

#define gpio_init_default() {}

#define gpio_clear(pin) {}
#define gpio_set(pin) {}

#define gpio_configure(pin, fun) {}

#define gpio_write_word(addr, value) (0)
#define gpio_read_word(addr, value) (0)
#define gpio_write_pin(pin, val) (0)
#define gpio_read_pin(pin) (0)

#define gpio_get_status(pin, t_up, t_down) (0)
