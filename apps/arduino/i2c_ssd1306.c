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
#include <memory/memory.h>
#include <serial/serial.h>
#include <i2c/i2c.h>
#include <rtc/rtc.h>
#include <disp/display.h>

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

static void time_thread_proc( void *args ){
	struct i2c_adapter *adapter = atmega_i2c_get_adapter(0); 
	printk("i2c_scan: \r\n"); 
	for(int c = 0; c <= 0x7f; c++){
		if(i2c_device_exists(adapter, c)) printk("%02x ", c); 
		else printk("00 ");  
		if((c & 0x7) == 0x7) printk("\r\n"); 
	}
	printk("\r\n"); 

	struct display_device *disp = ssd1306_new(adapter, 0x3c); 
	if(!disp) {
		printk("No display device found!\r\n"); 
		return; 
	}
	
	int tick = 0; 
	for( ;; ){
		for(int c = 0; c < 10; c++){
			display_draw_pixel(disp, tick, c, 1); 
		}
		printk("TICK\r\n"); 
		msleep(1000); 
		tick++; 
	}
}

static void dim_thread_proc( void *args ){
	struct pwm_device *dev = atmega_pwm_get_device(); 
	if(!dev) return; 
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
	thread_t time_thread, dim_thread; 

	init_blink(); 
	_delay_ms(1000); 	
	
	thread_create(&time_thread, 200, 1, NULL, time_thread_proc, NULL);  
	thread_create(&dim_thread, 80, 1, NULL, dim_thread_proc, NULL);  

	schedule_start(); 

	// if all goes well, we never get here. 
    for( ;; );
}

