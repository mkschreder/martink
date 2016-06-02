#include "imx23_gpio.h"

struct imx23_board {
	pio_dev_t gpio; 
}; 

void imx23_init(struct imx23_board *self); 
