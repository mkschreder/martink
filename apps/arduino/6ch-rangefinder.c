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
#include <pwm/pwm.h>
#include <serial/serial.h>
#include <serial/atmega_uart.h>
#include <arch/avr/mega/time.h>
#include <kernel/time.h>

#define SENSOR_COUNT 2
#define READING_TIMEOUT_US 60000UL

static int16_t _readings[SENSOR_COUNT]; 
static timestamp_t _timestamps[SENSOR_COUNT]; 
static uint8_t _cur_sensor = 0; 
static timestamp_t _timeout = 0; 
static int _state = 0; 
static uint32_t _reading_count = 0; 

static void _do_measure(void){
	cli(); 

	_state = 0; 

	_cur_sensor++; 
	if(_cur_sensor == SENSOR_COUNT) _cur_sensor = 0; 

	_readings[_cur_sensor] = 200; 
	gpio_write_pin(GPIO_PB5, 1); 
	gpio_write_pin(GPIO_PB0 + _cur_sensor, 0); 
	_delay_us(5); 
	gpio_write_pin(GPIO_PB0 + _cur_sensor, 1);  
	_delay_us(10); 
	gpio_write_pin(GPIO_PB0 + _cur_sensor, 0); 
	gpio_write_pin(GPIO_PB5, 0); 
	_state = 1; 
	
	PCIFR = 0x07; // clear all interrupts
	sei(); 	
}

static void _pcint_handler(void *data){
	int up = gpio_read_pin(GPIO_PD2 + _cur_sensor); 
	if(!up){
		// falling edge is when we calc the time 
		_readings[_cur_sensor] = tsc_ticks_to_us(tsc_read() - _timestamps[_cur_sensor]) / 58; 	
		_reading_count++; 
		_timeout = 0; 
		_state = 0;  
	} else {
		// rising edge is start of measure
		_timestamps[_cur_sensor] = tsc_read(); 	
		_reading_count++; 
		_state = 2;  	
	}
}

int main(void){
	
	for(int c = 0; c < SENSOR_COUNT; c++){
		gpio_configure(GPIO_PB0 + c, GP_OUTPUT); 
		gpio_write_pin(GPIO_PB0 + c, 0); 
		gpio_configure(GPIO_PD2 + c, GP_INPUT); 
		gpio_register_pcint(GPIO_PD2 + c, &_pcint_handler, NULL); 
	}
	printk("6THSENSE\n"); 
	
	gpio_configure(GPIO_PB1, GP_OUTPUT); 
    for( ;; ){
		_do_measure(); 
		_delay_ms(60); 
		if(_cur_sensor == 0){
			int16_t readings[SENSOR_COUNT]; 
			cli(); 
			memcpy(readings, _readings, sizeof(_readings[0]) * SENSOR_COUNT); 
			sei(); 

		cli(); 

		// dirty way to reset all the sensors just to make sure they never time out (sensor BUG!)
		for(int c = 0; c < SENSOR_COUNT; c++){
			_timestamps[c] = tsc_read(); 
			gpio_configure(GPIO_PD2 + c, GP_OUTPUT); 
			gpio_write_pin(GPIO_PD2 + c, 0); 
		}
		_delay_ms(1); 
		for(int c = 0; c < SENSOR_COUNT; c++){
			gpio_write_pin(GPIO_PD2 + c, 1); 
			gpio_configure(GPIO_PD2 + c, GP_INPUT); 
		}
		PCIFR = 0x07; // clear all pending pcint interrupts

		sei(); 


			printk("%lu ", _reading_count); 
			for(int c = 0; c < SENSOR_COUNT; c++){
				if(c != 0) printk(" "); 
				printk("%d", readings[c]); 
			}
			printk("\n"); 
		}
	}
}

