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

	struct memory_device *at24 = at24_device_new(adapter, 0x57); 
	if(!at24) { 
		printk("No memory found!\r\n"); 
		return; 
	}

	struct rtc_device *rtc = ds3231_new(adapter, 0x68); 
	if(!rtc) {
		printk("No rtc device found!\r\n"); 
		return; 
	}

	static char buf[8] = {'a','b','c','d',0,0,0,0}; 
	static struct rtc_date_time time; 

	for( ;; ){
		// uncomment to try writing
		//if(memory_device_write(at24, 0, buf, sizeof(buf)) < 0) printk("NOWRITE\r\n"); 
		//memset(buf, 0, sizeof(buf)); 
		if(memory_device_read(at24, 0, buf, sizeof(buf)) < 0) printk("NOREAD\r\n");  
		if(rtc_get_datetime(rtc, &time, 0) < 0) printk("NOTIME\r\n"); 
		printk("time: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.year, time.month, time.date, time.hour, time.minute, time.second); 
		for(size_t c = 0; c < sizeof(buf); c++) printk("%02x", buf[c]); 
		printk("\r\n"); 
		init_blink(); 
		msleep(1000); 
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
	
	thread_create(&time_thread, NULL, time_thread_proc, NULL);  
	thread_create(&dim_thread, NULL, dim_thread_proc, NULL);  

	schedule_start(); 

	// if all goes well, we never get here. 
    for( ;; );
}

