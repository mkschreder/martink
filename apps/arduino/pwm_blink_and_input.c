/*

Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

Arduino threaded example with two threads: 

- Thread1: dim led on PD6 using Atmega PWM
- Thread2: listen on uart for a number. Output same number of blinks on pin 13
  led when number received. 

Both tasks run as separate switched tasks so the first led can continue to be
dimmed while user is typing and while the second led starts blinking. 
*/ 

#include <arch/soc.h>
#include <kernel/mt.h>
#include <kernel/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <adc/adc.h>
#include <gpio/gpio.h>
#include <gpio/atmega_gpio.h>
#include <pwm/pwm.h>
#include <serial/serial.h>

// this is just a static blink to signal that everything is working
static void init_blink(void){
	DDRB |= (1 << 5); 
	for(int c = 0; c < 5; c++){
		PORTB |= (1 << 5); 
		_delay_ms(50); 
		PORTB &= ~(1 << 5); 
		_delay_ms(50); 
	}
}

static void blink_thread_proc( void *args ){
	for( ;; ){
		char ch[2] = {0}; 
		printk("enter number for tx: "); 	
		serial_read(_default_system_console, ch, 1); 	
		printk("tx got %d\r\n", atoi(ch)); 	
		//uint16_t adc = adc_read_channel(adc_get_device(0), 0); 
		//printk("adc value: %d\r\n", adc); 
		for(int c = 0; c < atoi(ch); c++){
			PORTB |= (1 << 5); 
			// this is real sleep, not a busy wait
			// task will not run at all while it sleeps 
			msleep(50); 
			PORTB &= ~(1 << 5); 
			msleep(50); 
		}
	}
}

static void dim_thread_proc( void *args ){
	struct pwm_device *dev = atmega_pwm_get_device(); 
	gpio_configure(GPIO_PD6, GP_OUTPUT); 
	for( ;; ){
		pwm_set_output(dev, 0, 1); 	
		for(int c = 0; c < 10; c++){
			pwm_set_period(dev, 0, 20 * c);  
			msleep(100); 
		}
		for(int c = 10; c >= 0; c--){
			pwm_set_period(dev, 0, 20 * c);  
			msleep(100); 
		}
		pwm_set_period(dev, 0, 0);  
		msleep(1000); 
	}
}

int main(void){
	thread_t blink_thread, dim_thread; 

	init_blink(); 
	_delay_ms(1000); 	

	thread_create(&blink_thread, 32, 0, "blink", blink_thread_proc, NULL);  
	thread_create(&dim_thread, 32, 0, "dim", dim_thread_proc, NULL);  

	schedule_start(); 

	// if all goes well, we never get here. 
    for( ;; );
}

