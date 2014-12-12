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

#include "hcsr04.h"

//values for state
#define ST_IDLE 0
#define ST_SENDING_START_PULSE 1
#define ST_WAITING_RESPONSE_PULSE 2
#define ST_MEASURING_RESPONSE_PULSE 3
#define ST_WAITING_ECHO_FADING_AWAY 4

volatile uint8_t state = ST_IDLE;

//values for oper_mode
#define SINGLE_SHOT 0
#define CONTINUOUS 1

volatile uint8_t oper_mode = CONTINUOUS;

//the result of the latest measurement
volatile uint16_t resp_pulse_length;

int send_pulse(void);

/**
 * Initialize the timer for a new measurement cycle
 */
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

/**
 * This int handler is called twice during measurement cycle
 *  - when it is time to generate the trailing edge of the start pulse
 *  - when the measurement cycle ends
 */
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

/**
 * Initialize the I/O pins for a new measurement cycle
 */
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

/**
 * This int handler is called twice during measurement cycle
 *  - when it is time to generate the trailing edge of the start pulse
 *  - when the measurement cycle ends
 */
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

int send_pulse(void)
{
	if (state != ST_IDLE)
		return 0;

	pin_init();
	PORTC |= (1 << PC3);	// the leading edge of the pulse
	timer_init();

	state = ST_SENDING_START_PULSE;

	return 1;
}

int hcsr04_send_pulse(void)
{
	if (oper_mode == CONTINUOUS)
		return 0;

	return send_pulse();
}

int hcsr04_start_continuous_meas(void)
{
	int ret_val = send_pulse();

	if (ret_val)
		oper_mode = CONTINUOUS;

	return ret_val;
}

void hcsr04_stop_continuous_meas(void)
{
	oper_mode = SINGLE_SHOT;
}

uint16_t hcsr04_get_pulse_length(void)
{
	uint16_t ret_val;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		ret_val = resp_pulse_length;
	}
	return ret_val;
}

void hcsr04_init(void){
	gpio_set_function(GPIO_PA10, GP_PULSE_IN); 
	gpio_read(GPIO_PA10); // returns pulse width in usec. 
	
	send_pulse(); 
	//timer_init(); 
	//pin_init(); 
}

uint16_t hcsr04_get_distance_in_cm(void)
{
	uint16_t tmp_pulse_length;
	ATOMIC_BLOCK(ATOMIC_FORCEON) {
		tmp_pulse_length = resp_pulse_length;
	}
	if (tmp_pulse_length == HCSR04_MEAS_FAIL)
		return tmp_pulse_length;

	// 36 pulses per 10 cm, when clock divider is 256 in TCCR1B
	return (10 * tmp_pulse_length) / 36;
}
