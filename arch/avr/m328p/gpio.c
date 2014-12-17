/**
	This is a fast GPIO implementation for ATMega

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

#include <arch/soc.h>
#include <string.h>

static uint8_t _dummyPR = 0; 
const struct pin_decl gPinPorts[4] = {
	{&_dummyPR, &_dummyPR, &_dummyPR },
	{&PORTB, &PINB, &DDRB},
	{&PORTC, &PINC, &DDRC},
	{&PORTD, &PIND, &DDRD},
};

volatile struct pin_state gPinState[GPIO_COUNT - GPIO_PB0] = {0}; 

ISR(PCINT0_vect){
	timeout_t time = time_get_clock(); 
	static uint8_t prev[3] = {0xff, 0xff, 0xff}; 
	uint8_t current[3] = {PINB, PINC, PIND}; 
	uint8_t changed[3] = {
		prev[0] ^ current[0], 
		prev[1] ^ current[1], 
		prev[2] ^ current[2]
	}; 
	memcpy(prev, current, 3); 
	uint8_t pin_id = 0; 
	for(int reg = 0; reg < 3; reg++){
		for(int bit = 0; bit < 8; bit++){
			if(changed[reg] & _BV(bit)){
				volatile struct pin_state *st = &gPinState[pin_id]; 
				if(current[reg] & _BV(bit)){
					st->t_up = time; 
					st->status = GP_WENT_HIGH; 
				} else {
					st->t_down = time; 
					st->status = GP_WENT_LOW; 
				}
			}
			pin_id++; 
		}
	}
}

ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect)); 

ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));
