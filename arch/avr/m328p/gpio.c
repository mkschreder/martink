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

#if defined(CONFIG_GPIO_PIN_STATES)
	// PIN state recording for gpio pins
	volatile struct pin_state gPinState[GPIO_COUNT - GPIO_PB0] = {{0}}; 

	uint8_t gpio_get_status(gpio_pin_t pin, 
		timestamp_t *ch_up, timestamp_t *ch_down){
		pin = (pin < GPIO_PB0)?GPIO_PB0:((pin > GPIO_PD7)?GPIO_PD7:pin); 
		uint8_t ret = 0;
		volatile struct pin_state *st = &gPinState[pin - GPIO_PB0]; 
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			ret = st->status; 
			*ch_up = st->t_up; 
			*ch_down = st->t_down; 
			st->status = 0; 
		}
		return ret; 
	}
	
	ISR(PCINT0_vect){
		timestamp_t time = timestamp_now(); 
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
#endif

// a pseudorandom function to make results even more "random"
static uint16_t _fx_rand(void)
{
	static uint16_t lfsr = 0xACE1u;
	static uint16_t bit;
	bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
	return lfsr =  (lfsr >> 1) | (bit << 15);
}
// initializes the standard random number generator from a floating pin
uint32_t gpio_read_prng(gpio_pin_t pin){
	// save pin state
	uint8_t dreg = DREG(pin);
	uint8_t ireg = IREG(pin);
	uint8_t oreg = OREG(pin);
	
	uint8_t pidx = PIDX(pin); 

	// set pin as input and disable internal pullup
	DREG(pin) &= ~_BV(pidx);
	IREG(pin) &= ~_BV(pidx);
	OREG(pin) &= ~_BV(pidx);
	
	// do a test to see if the pin really is floating (ie no external pullups etc.)
	{
		uint16_t timeout = 5000; 
		uint8_t d = IREG(pin) & _BV(PIDX(pin));
		while((d == (IREG(pin) & _BV(PIDX(pin)))) && timeout--){
			_delay_us(1); 
		}
		// pin is not floating because value did not change randomly
		if(timeout == 0) {
			// restore old values
			OREG(pin) = oreg;
			IREG(pin) = ireg;
			DREG(pin) = dreg;
			return 0; 
		}
	}
	
	uint8_t size = sizeof(uint32_t); 
	uint32_t sr = 0; 
	uint8_t *buf = (uint8_t*)&sr; 
	for(int byte = 0; byte < size; byte++){
		buf[byte] = 0; 
		uint16_t timeout = 0xff;
		for(int c = 0; c < 8 * sizeof(char); c++){
			uint16_t x = 0;
			uint8_t d = IREG(pin) & _BV(PIDX(pin));
			while((d == (IREG(pin) & _BV(PIDX(pin)))) && timeout--){
				x += _fx_rand();
			}
			timeout = x & 0xff; 
			buf[size] |= ((x & 1) << c); 
		}
	}
	
	// restore old values
	OREG(pin) = oreg;
	IREG(pin) = ireg;
	DREG(pin) = dreg;
	
	// we should never return 0 because it signifies error
	return (sr == 0)?1:sr; 
}
