/*

Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>
License: GPLv3

*/

#include <arch/soc.h>
#include <kernel/mt.h>
#include <kernel/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <adc/adc.h>
#include <gpio/gpio.h>
#include <gpio/atmega_gpio.h>
#include <spi/atmega_spi.h>
#include <pwm/pwm.h>
#include <serial/serial.h>
#include <serial/atmega_uart.h>
#include <disp/max7456.h>

static void app_main(void *arg){
	struct max7456 osd; 
	struct gpio_adapter *gpio = atmega_gpio_get_adapter(); 
	struct spi_adapter *spi = atmega_spi_get_adapter(); 
	
	max7456_init(&osd, spi, gpio, GPIO_PD6); 
	uint8_t ch = 0; 
	for(int y = 0; y < 16; y++){
		for(int x = 0; x < 30; x++){
			max7456_write_char_at(&osd, x, y, ch++); 
		}
	}
			
	for(;;){
		msleep(100); 
	}
}

int main(void){
	thread_t app_thread; 

	_delay_ms(100); 
	DDRB |= (1 << 5); 
	for(int c = 0; c < 5; c++){
		PORTB |= (1 << 5); 
		_delay_ms(50); 
		PORTB &= ~(1 << 5); 
		_delay_ms(50); 
	}

	thread_create(&app_thread, 512, 1, "minoposd", app_main, NULL); 

	schedule_start(); 
	
	for(;;); 

	return 0; 
}
