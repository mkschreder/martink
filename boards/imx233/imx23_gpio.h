#include <kernel/dev/parallel.h>

struct imx23_gpio {
	int *mmap; 
	struct pio_if *api; 
}; 

void imx23_gpio_init(struct imx23_gpio *self);
pio_dev_t imx23_gpio_get_parallel_interface(struct imx23_gpio *self); 
