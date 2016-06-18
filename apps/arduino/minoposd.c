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
#include <tty/vt100.h>
#include <serial/atmega_uart.h>

static void app_main(void *arg){
	struct max7456 osd; 
	struct vt100 vt100; 
	struct gpio_adapter *gpio = atmega_gpio_get_adapter(); 
	struct spi_adapter *spi = atmega_spi_get_adapter(); 
	struct serial_device *uart = atmega_uart_get_adapter(); 

	serial_set_baud(uart, 57600); 

	max7456_init(&osd, spi, gpio, GPIO_PD6); 
	vt100_init(&vt100, max7456_to_tty_device(&osd)); 

	//serial_printf(vt100_to_serial_device(&vt100), "Hello World!\nNext Line\n"); 
	for(;;){
		char ch; 
		serial_read(uart, &ch, 1); 
		serial_write(uart, &ch, 1); 
		serial_write(vt100_to_serial_device(&vt100), &ch, 1); 
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
