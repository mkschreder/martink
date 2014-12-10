#include <avr/io.h>
#include <stdlib.h>

#include "random.h"

//#define RANDOM_PORT PORTD

uint16_t _fx_rand(void)
{
	static uint16_t lfsr = 0xACE1u;
	static uint16_t bit;
	bit  = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5) ) & 1;
	return lfsr =  (lfsr >> 1) | (bit << 15);
}
  /*
void fx_random(char *buf, uint16_t size){
	// set pin as input
	RANDOM_DDR &= ~_BV(RANDOM_BIT);
	RANDOM_PORT &= ~_BV(RANDOM_BIT);

	while(size){
		size--;
		buf[size] = 0; 
		uint16_t timeout = 0xff;
		for(int c = 0; c < 8 * sizeof(char); c++){
			uint16_t x = 0;
			uint8_t d = RANDOM_PIN & _BV(RANDOM_BIT);
			while((d == (RANDOM_PIN & _BV(RANDOM_BIT))) && timeout--){
				x += _fx_rand();
			}
			timeout = x & 0xff; 
			buf[size] |= ((x & 1) << c); 
		}
	}
}
*/
#define RANDOM_PIN *inport
#define RANDOM_DDR *ddrport

// initializes the standard random number generator from a floating pin
void prng_init(volatile uint8_t *inport, volatile uint8_t *ddrport, uint8_t rndpin){
	// set pin as input
	RANDOM_DDR &= ~_BV(rndpin);
	RANDOM_PIN &= ~_BV(rndpin);
	
	unsigned int size = sizeof(unsigned int); 
	unsigned int sr = 0; 
	uint8_t *buf = (uint8_t*)&sr; 
	while(size){
		size--;
		buf[size] = 0; 
		uint16_t timeout = 0xff;
		for(int c = 0; c < 8 * sizeof(char); c++){
			uint16_t x = 0;
			uint8_t d = RANDOM_PIN & _BV(rndpin);
			while((d == (RANDOM_PIN & _BV(rndpin))) && timeout--){
				x += _fx_rand();
			}
			timeout = x & 0xff; 
			buf[size] |= ((x & 1) << c); 
		}
	}
	srand(sr); 
}

