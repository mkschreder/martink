#include <arch/soc.h>

static uint8_t _dummyPR = 0; 
const struct pin_decl gPinPorts[4] = {
	{&_dummyPR, &_dummyPR, &_dummyPR },
	{&PORTB, &PINB, &DDRB},
	{&PORTC, &PINC, &DDRC},
	{&PORTD, &PIND, &DDRD},
};
