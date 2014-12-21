#pragma once

#define PF(n,x) GPIO_P##n##x
#define P(n,x,y) GPIO_P##n##x##y

#define PDEF(n) \
	PF(n,0), PF(n,1), PF(n,2), PF(n,3), PF(n,4),\
	PF(n,5), PF(n,6), PF(n,7), PF(n,8), PF(n,9),\
	P(n,1,0), P(n,1,1), P(n,1,2), P(n,1,3), P(n,1,4),\
	P(n,1,5), P(n,1,6), P(n,1,7), P(n,1,8), P(n,1,9), \
	P(n,2,0), P(n,2,1), P(n,2,2), P(n,2,3), P(n,2,4),\
	P(n,2,5), P(n,2,6), P(n,2,7), P(n,2,8), P(n,2,9), \
	P(n,3,0), P(n,3,1)
	
enum {
	GPIO_NONE = 0, 
	PDEF(A), PDEF(B), PDEF(C), PDEF(D)
};

#undef PF
#undef P
#undef PDEF

#define PIO_BANK(n) ((Pio*[]){PIOA, PIOB, PIOC, PIOD}[(n) & 0x03])
#define PPIO(pin) (PIO_BANK((pin - 1) >> 5))
#define PIDX(pin) ((uint32_t)(pin - 1) & 0x1f)
#define PMASK(pin) (1 << PIDX(pin))
//static inline uint8_t PIDX(uint16_t pin) {
//	return ((1 << ((uint32_t)(pin - 1) & 0x1fL))

#define gpio_init() (\
	gpio_init_pin_states()\
)

#define gpio_configure(pin, fun) (\
	((fun) & GP_OUTPUT)\
		?PIO_Configure(PPIO(pin), PIO_OUTPUT_0, PMASK(pin), PIO_DEFAULT)\
		:PIO_Configure(PPIO(pin), PIO_INPUT, PMASK(pin), PIO_DEFAULT), \
	(((fun) & GP_PULL) && ((fun) & GP_PULLUP))\
		?PIO_PullUp(PPIO(pin), PMASK(pin), 1)\
		:PIO_PullUp(PPIO(pin), PMASK(pin), 0), \
	((fun) & GP_PCINT)\
		?(PPIO(pin)->PIO_IDR |= PMASK(pin))\
		:(PPIO(pin)->PIO_IDR &= ~PMASK(pin))\
)


#define gpio_write_word(addr, value) (\
	PIO_Set(PIO_BANK(addr), value), 0 \
)

#define gpio_read_word(addr, value) (uint32_t)(\
	*value = PIO_BANK(addr)->PIO_PDSR\
)

#define gpio_write_pin(pin, val) (\
	PIO_Set(PPIO(pin), PMASK(pin))\
)

#define gpio_read_pin(pin) (\
	PIO_Get(PPIO(pin), PIO_INPUT, PMASK(pin)) \
)

#define gpio_clear(pin) gpio_write_pin(pin, 0)
#define gpio_set(pin) gpio_write_pin(pin, 1)

#define gpio_get_status(pin, t_up, t_down) (0)
/*
// evaluates to bit that has been enabled or -1 if invalid pcint
#define gpio_enable_pcint(pin) (\
	((pin) >= GPIO_PB0 && (pin) <= GPIO_PB7)\
		?(PCICR |= _BV(PCINT0), PCMSK0 = PCMSK0 | _BV((pin) - GPIO_PB0))\
		:((pin) >= GPIO_PC0 && (pin) <= GPIO_PC7_NC)\
			?(PCICR |= _BV(PCINT1), PCMSK1 = PCMSK1 | _BV((pin) - GPIO_PB0))\
			:((pin) >= GPIO_PD0 && (pin) <= GPIO_PD7)\
				?(PCICR |= _BV(PCINT2), PCMSK2 = PCMSK2 | _BV((pin) - GPIO_PD0))\
				:(-1)\
)

#define gpio_disable_pcint(pin) (\
	((pin) >= GPIO_PB0 && (pin) <= GPIO_PB7)\
		?(PCICR &= ~_BV(PCINT0), PCMSK0 = PCMSK0 & ~_BV((pin) - GPIO_PB0))\
		:((pin) >= GPIO_PC0 && (pin) <= GPIO_PC7)\
			?(PCICR &= ~_BV(PCINT1), PCMSK1 = PCMSK1 & ~ _BV((pin) - GPIO_PB0))\
			:((pin) >= GPIO_PD0 && (pin) <= GPIO_PD7)\
				?(PCICR &= ~_BV(PCINT2), PCMSK2 = PCMSK2 & ~ _BV((pin) - GPIO_PD0))\
				:(-1)\
)
*/
