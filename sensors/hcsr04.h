#ifndef HCSR04_H
#define HCSR04_H

#define HCSR04_MEAS_FAIL 0xFFFF

struct hcsr04 {
	struct parallel_interface *gpio;
	gpio_pin_t trigger_pin, echo_pin;
	uint8_t state;
	int16_t distance; 
};

void hcsr04_init(struct hcsr04 *self, struct parallel_interface *gpio,
	gpio_pin_t trigger_pin, gpio_pin_t echo_pin); 
int16_t hcsr04_read_distance_in_cm(struct hcsr04 *self); 

#endif
