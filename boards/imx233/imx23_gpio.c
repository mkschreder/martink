#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <arch/soc.h>

#include "imx23_gpio.h"

#define GPIO_BASE 0x80018000
#define GPIO_WRITE_PIN(gpio,value) GPIO_WRITE((gpio)>>5, (gpio)&31, value)
#define GPIO_WRITE(bank,pin,value) (self->mmap[0x140+((bank)<<2)+((value)?1:2)] = 1<<(pin))
#define GPIO_READ_PIN(gpio) GPIO_READ((gpio)>>5, (gpio)&31)
#define GPIO_READ(bank,pin) ((self->mmap[0x180+((bank)<<2)] >> (pin)) & 1)

void imx23_gpio_init(struct imx23_gpio *self) {
	int fd;
	memset(self, 0, sizeof(struct imx23_gpio)); 
	
	fd = open("/dev/mem", O_RDWR);
	if( fd < 0 ) {
		ERROR("Unable to open /dev/mem\n");
		fd = 0;
	}

	self->mmap = mmap(0, 0xfff, PROT_READ|PROT_WRITE, MAP_SHARED, fd, GPIO_BASE);
	if( (void*)-1 == self->mmap) {
		ERROR("Unable to mmap file\n");
		self->mmap = 0;
	}
	if( -1 == close(fd))
		ERROR("Couldn't close file\n");
}
/*
int gpio_rd(long offset) {
	return gpio_mmap[offset/4];
}

void gpio_wr(long offset, long value) {
	gpio_mmap[offset/4] = value;
}

void gpio_output(int bank, int pin) {
	gpio_mmap[0x1C1 + (bank*4)] = 1 << pin;
}

void gpio_input(int bank, int pin) {
	gpio_mmap[0x1C2 + (bank*4)] = 1 << pin;
}
*/
static void _gpio_write_pin(pio_dev_t dev, uint16_t pin, uint8_t value){
	struct imx23_gpio *self = container_of(dev, struct imx23_gpio, api); 
	if(!self->mmap) return; 
	GPIO_WRITE_PIN(pin, value); 
	//self->mmap[pin/4] = value;
}

static uint8_t _gpio_read_pin(pio_dev_t dev, uint16_t pin){
	struct imx23_gpio *self = container_of(dev, struct imx23_gpio, api); 
	if(!self->mmap) return 0; 
	return GPIO_READ_PIN(pin); 
	//return self->mmap[pin/4];
}

static uint8_t _gpio_configure_pin(pio_dev_t dev, uint16_t pin, uint16_t flags){
	struct imx23_gpio *self = container_of(dev, struct imx23_gpio, api); 
	if(!self->mmap) return 0; 
	
	int bank = pin >> 5; // bank is pin % 32
	pin = pin & 0x1f; // update pin to be 0-31 in bank
	if(flags & GP_OUTPUT)
		self->mmap[0x1C1 + (bank*4)] = 1 << pin;
	if(flags & GP_INPUT) 
		self->mmap[0x1C2 + (bank*4)] = 1 << pin;
	return 0; 
}

static uint8_t _gpio_write_word(pio_dev_t dev, uint16_t addr, uint32_t value){
	(void)(dev); 
	(void)addr; 
	(void)value; 
	return 0; 
	//return gpio_write_word(addr, value);
}

static uint8_t _gpio_read_word(pio_dev_t dev, uint16_t addr, uint32_t *output){
	(void)(dev); 
	(void)addr; 
	(void)output; 
	return 0; 
	//return gpio_read_word(addr, output);
}

pio_dev_t imx23_gpio_get_parallel_interface(struct imx23_gpio *self){
	static struct pio_if pio;
	if(!self->api){
		pio = (struct pio_if){
			.configure_pin = _gpio_configure_pin, 
			.write_pin = 		_gpio_write_pin,
			.read_pin = 		_gpio_read_pin,
			.write_word = 	_gpio_write_word,
			.read_word = 		_gpio_read_word,
			//.get_pin_status = _gpio_get_pin_status
		};
		self->api = &pio; 
	}
	return &self->api; 
}
