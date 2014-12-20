/* -*- mode: c; c-file-style: "linux" -*-
 *  vi: set shiftwidth=8 tabstop=8 noexpandtab:
 *
 *  Copyright 2012 Elovalo project group 
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Device driver for HC-SR04 ultrasound module
 */

#include <stdio.h>
#include <inttypes.h>

#include <arch/soc.h>

#include "hcsr04.h"

//values for state
#define ST_IDLE 0
#define ST_PULSE_SENT 1
/*
volatile uint8_t state = ST_IDLE;

//values for oper_mode
#define SINGLE_SHOT 0
#define CONTINUOUS 1

volatile uint8_t oper_mode = CONTINUOUS;

//the result of the latest measurement
volatile uint16_t resp_pulse_length;

int send_pulse(void);

void timer_init(void)
{
	//TCCR1A &= ~(1<<WGM11 | 1<<WGM10); //no need to do this, bits are cleared by default
	//TCCR1B &= ~(1<<WGM13 | 1<<WGM12); //no need to do this, bits are cleared by default
	TCCR1B |= (1 << WGM12);	// sets compare and reset to "top" mode
	TCCR1B |= (1 << CS10);	// set clock divider to 1

	OCR1A = 184;		// set "top" for 10 us (16 MHz sysclock in use)
	// TODO: the actual pulse length is 10 ms! Why?
	TCNT1 = 0;
	TIFR1 |= (1 << OCF1A);	// clear possible pending int
	TIMSK1 |= (1 << OCIE1A);	// enable int
}

ISR(TIMER1_COMPA_vect)
{

	if (state == ST_SENDING_START_PULSE) {

		PORTC &= ~(1 << PC3);	// force trailing edge of the pulse
		
		//re-init TIMER1 for measuring response pulse length
		TCCR1B &= ~(1 << CS12 | 1 << CS11 | 1 << CS10);	// stop timer
		OCR1A = 6000;	// set "top" for 96 ms (16 MHz sysclock in use)
		TCNT1 = 0;
		TCCR1B |= (1 << CS12);	// start timer, set clock divider to 256

		state = ST_WAITING_RESPONSE_PULSE;
	} else {
		//normally the state should be ST_WAITING_ECHO_FADING_AWAY here
		//but clear state machine also in all other cases

		if (state == ST_MEASURING_RESPONSE_PULSE) {
			// This situation happens when there is no
			// object in the beam area
			resp_pulse_length = HCSR04_MEAS_FAIL;
		}

		TIMSK1 &= ~(1 << OCIE1A);	// disable timer int
		PCICR &= ~(1 << PCIE1);	// disable PIN Change int
		state = ST_IDLE;

		if (oper_mode == CONTINUOUS)
			send_pulse();
	}
}

void pin_init(void)
{
	DDRC |= (1 << PC3);	//output
	DDRC &= ~(1 << PC2);	//input

	//Enable PIN Change Interrupt 1 - This enables interrupts on pins
	//PCINT14...8, see doc8161.pdf Rev. 8161D – 10/09, ch 12
	PCICR |= (1 << PCIE1);

	//Set the mask on Pin change interrupt 1 so that only PCINT12 (PC4) triggers
	//the interrupt. see doc8161.pdf Rev. 8161D – 10/09, ch 12.2.1
	PCMSK1 |= (1 << PCINT10);
	return;
}

ISR(PCINT1_vect)
{
	register uint8_t leading_edge = PINC & (1 << PINC2);
	if (state == ST_WAITING_RESPONSE_PULSE) {
		if (leading_edge) {
			TCNT1 = 0; //restart counting
			state = ST_MEASURING_RESPONSE_PULSE;
		}
		//else{
		//trailing edge, just fall through
		//}
	} else if (state == ST_MEASURING_RESPONSE_PULSE) {
		if (!leading_edge) {
			resp_pulse_length = TCNT1;
			PCICR &= ~(1 << PCIE1);	//Disable PIN Change Interrupt 1
			state = ST_WAITING_ECHO_FADING_AWAY;
		}
		//else{
		//leading edge, just fall through
		//}
	}
	//else{
	//should not happen
	//}
}
*/

static uint8_t hcsr04_send_pulse(struct hcsr04 *self)
{
	if (self->state != ST_IDLE)
		return 0;

	self->gpio->write_pin(self->gpio, self->trigger_pin, 1);
	delay_us(10);
	self->gpio->write_pin(self->gpio, self->trigger_pin, 0);
	
	self->state = ST_PULSE_SENT;

	return 1;
}

void hcsr04_init(struct hcsr04 *self, struct parallel_interface *gpio,
	gpio_pin_t trigger_pin, gpio_pin_t echo_pin){
	self->gpio = gpio;
	self->trigger_pin = trigger_pin;
	self->echo_pin = echo_pin;

	self->gpio->configure_pin(self->gpio, self->trigger_pin, GP_OUTPUT);
	self->gpio->configure_pin(self->gpio, self->trigger_pin,
		GP_INPUT | GP_PCINT); 

	self->state = ST_IDLE;
	self->distance = -1;
	
	hcsr04_send_pulse(self); 
}

uint8_t hcsr04_check_response(struct hcsr04 *self){
	timestamp_t t_up, t_down; 
	uint8_t status = self->gpio->get_pin_status(self->gpio, self->echo_pin, &t_up, &t_down);
	if(status == GP_WENT_LOW){
		timestamp_t us = timestamp_ticks_to_us(t_down - t_up);
		// convert to cm 
		self->distance = (uint16_t)(us * 0.000001 * 34029.0f);
		self->state = ST_IDLE; 
		return 1;
	}
	return 0; 
}

int16_t hcsr04_read_distance_in_cm(struct hcsr04 *self)
{
	if(self->state == ST_IDLE){
		hcsr04_send_pulse(self);
	}
	uint32_t timeout = 50000; 
	while(!hcsr04_check_response(self)){
		timeout--;
		if(timeout == 0) return -1; 
		delay_us(10);
	}
	return self->distance;
}
