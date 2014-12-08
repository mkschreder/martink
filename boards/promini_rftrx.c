/**
	This file is part of martink project.

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#include "promini_rftrx.h"
#include <kernel.h>

#ifdef BOARD_PROMINI_RFTRX

#define DEVICE_TWI_ADDRESS 0x88

struct board {
	uint16_t pwm[6]; 
	uint16_t adc[6]; 
	timeout_t ch_timeout[6]; 
	timeout_t pwm_timeout; 
	timeout_t signal_timeout; 
	uint16_t pwm_pulse_delay_us; 
	volatile uint8_t pwm_lock; 
}; 

static struct board _brd; 
static struct board *brd = &_brd; 
static struct uart uart;

void brd_init(void){
	// radio ce and cs pins
	DDRB |= _BV(0) | _BV(1); 
	PORTB |= _BV(0) | _BV(1); 
	
	brd->pwm_pulse_delay_us = 2500; 
	brd->pwm_lock = 0; 
	
	uint8_t bits = _BV(2) | _BV(3) | _BV(4) | _BV(5) | _BV(6) | _BV(7); 
	DDRD |= bits; 
	PORTD &= ~bits; 
	
	uart_init(&uart, UART_BAUD_SELECT(38400, F_CPU));
	uart_printf(PSTR("booting..\n")); 
	
	adc_init(); 
	time_init(); 
	twi_slave_init(DEVICE_TWI_ADDRESS); 
	
	rfnet_init(&PORTB, &DDRB, PB0, PB1); 
	/*
	TCCR0B = _BV(CS01) | _BV(CS00);
	TIMSK0 |= _BV(TOIE0); 
	TCNT0 = 0; 
	*/
	sei(); 
}

void brd_process_events(void){
	if(timeout_expired(brd->pwm_timeout)){
		brd->pwm_timeout = timeout_from_now(brd->pwm_pulse_delay_us); 
		
		for(int c = 0; c < 6; c++){
			// skip channels that are zero
			if(brd->pwm[c] == 0) continue; 
			brd->ch_timeout[c] = timeout_from_now(brd->pwm[c]); 
			//PORTD |= _BV(2+c); 
		}
		for(;;){
			uint8_t done = 1; 
			for(int c = 0; c < 6; c++){
				if(timeout_expired(brd->ch_timeout[c])){
					//PORTD &= ~_BV(2+c); 
				} else {
					done = 0; 
				}
			}
			if(done) break; 
		}
	}
	if(timeout_expired(brd->signal_timeout)){
		for(int c = 0; c < 6; c++)
			set_pin(PWM_OUT_0 + c, 0); 
	}
}

/*
ISR(TIMER0_OVF_vect)
{
	sei(); 
	brd_process_events(&pwm); 
}*/

void set_pin(uint8_t pin, uint16_t value){
	if(pin >= PWM_OUT_0 && pin <= PWM_OUT_5){
		uint8_t idx = pin - PWM_OUT_0; 
		if(idx > 5) return; 
		brd->pwm[idx] = value; 
		brd->signal_timeout = timeout_from_now(1000000); 
	}
}

uint16_t get_pin(uint8_t pin){
	if(pin >= ANALOG_IN_0 && pin <= ANALOG_IN_5){
		uint8_t idx = pin - ANALOG_IN_0; 
		if(idx > 5) return 0; 
		return brd->adc[idx]; 
	} 
	return 0; 
}

#endif
