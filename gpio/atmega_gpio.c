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
#include <kernel/mt.h>
#include <kernel/time.h>
#include <stdlib.h>

#include "gpio.h"
#include "atmega_gpio.h"
#include <string.h>

struct pin_decl {
	volatile uint8_t *out_reg;
	volatile uint8_t *in_reg;
	volatile uint8_t *ddr_reg;
};

static uint8_t _dummyPR = 0; 
static const struct pin_decl gPinPorts[4] = {
	{&_dummyPR, &_dummyPR, &_dummyPR },
	{&PORTB, &PINB, &DDRB},
	{&PORTC, &PINC, &DDRC},
	{&PORTD, &PIND, &DDRD},
};

//volatile struct pin_state *_pin_states[GPIO_COUNT - GPIO_PB0] = {0}; 
//static volatile uint8_t _pins_enabled[4] = {0}; 

/*
uint8_t gpio_pin_busy(gpio_pin_t pin){
	if(pin < GPIO_PB0 || pin > GPIO_PD7) return 0; 
	pin = (pin - GPIO_PB0); 
	return _pin_states[pin] != 0; //_pins_enabled[(pin >> 3) & 0x3] & (1 << (pin & 0x07)); 
}

int8_t gpio_start_read(gpio_pin_t pin, volatile struct pin_state *state, uint8_t flags){
	if(pin < GPIO_PB0 || pin > GPIO_PD7 || gpio_pin_busy(pin)) return -1; 
	pin = pin - GPIO_PB0; 
	// reset state flags
	state->status = 0; 
	state->flags = flags;
	// enable pin for ISR
	_pin_states[pin] = state; 
	// mark pin as enabled for the ISR
	//_pins_enabled[(pin >> 3)] |= (1 << (pin & 0x07)); 
	 
	return 0; 
}
*/
#if 0
	static void PCINT_vect(void){
		timestamp_t time = timestamp_now(); 
		static uint8_t prev[3] = {0xff, 0xff, 0xff}; 
		uint8_t current[3] = {PINB, PINC, PIND}; 
		uint8_t changed[3] = {
			prev[0] ^ current[0], 
			prev[1] ^ current[1], 
			prev[2] ^ current[2]
		}; 
		prev[0] = current[0]; prev[1] = current[1]; prev[2] = current[2]; 
		uint8_t pin_id = 0; 
		for(int reg = 0; reg < 3; reg++){
			for(int bit = 0; bit < 8; bit++){
				if(	_pin_states[pin_id] != 0 && (changed[reg] & _BV(bit)) ){
					volatile struct pin_state *st = _pin_states[pin_id]; 
					if(current[reg] & _BV(bit)){ // if pin went high
						st->t_up = time; 
						st->status |= GP_READ_EDGE_P; 
						// we are done if measuring neg pulse and got neg edge
						if(	(st->status & GP_READ_EDGE_N && st->flags & GP_READ_PULSE_N) || 
							(st->flags & GP_READ_EDGE_P && !(st->flags & GP_READ_PULSE_P))){
							_pin_states[pin_id] = 0; 
						}
					} else { // if bit went low
						st->t_down = time; 
						st->status |= GP_READ_EDGE_N; 
						// check if we are done
						if(	(st->status & GP_READ_EDGE_P && st->flags & GP_READ_PULSE_P) ||
							(st->flags & GP_READ_EDGE_N && !(st->flags & GP_READ_PULSE_N))){
							_pin_states[pin_id] = 0; 
						}
					}
				}
				pin_id++; 
			}
		}
	}


	// PIN state recording for gpio pins
	/*volatile struct pin_state gPinState[GPIO_COUNT - GPIO_PB0] = {{0}}; 

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
	
	static void PCINT_vect(void){
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
	*/
	ISR(PCINT0_vect){
		PCINT_vect(); 
	}
	
	ISR(PCINT1_vect){
		PCINT_vect(); 
	}

	ISR(PCINT2_vect){
		PCINT_vect(); 
	}
#endif

enum {
	GP_READ_PULSE_P = (1 << 0), 
	GP_READ_PULSE_N = (1 << 1), 
	GP_READ_EDGE_P 	= (1 << 2), 
	GP_READ_EDGE_N 	= (1 << 3) 
}; 

struct pin_state {
	volatile timestamp_t t_up; 
	volatile timestamp_t t_down; 
	volatile uint8_t status; 
	volatile uint8_t flags; 
}; 

//extern volatile struct pin_state gPinState[GPIO_COUNT - GPIO_PB0]; 

#define RIDX(pin) 	((pin > 0)?(((pin - 1) & 0xf8) >> 3):0)
#define PIDX(pin) 	((pin > 0)?((pin - 1) & 0x07):0)
#define OREG(pin) 	*(gPinPorts[RIDX(pin)].out_reg)
#define IREG(pin) 	*(gPinPorts[RIDX(pin)].in_reg)
#define DREG(pin) 	*(gPinPorts[RIDX(pin)].ddr_reg)
#define RSET(reg, bit) (reg |= _BV(bit))
#define RCLR(reg, bit) (reg &= ~_BV(bit))

#if defined(CONFIG_GPIO_PIN_STATES)
#define gpio_init_pin_states() (timestamp_init())
#else
#define gpio_init_pin_states() (0)
#endif

#define gpio_init_default() (\
	gpio_init_pin_states()\
)
/*
void gpio_configure(gpio_pin_t pin, uint16_t conf); 
uint8_t gpio_write_word(uint8_t addr, uint32_t value);
uint8_t gpio_read_word(uint8_t addr, uint32_t *value);
void gpio_write_pin(gpio_pin_t pin, uint8_t val);
uint8_t gpio_read_pin(gpio_pin_t pin);

uint8_t gpio_pin_busy(gpio_pin_t pin);
int8_t gpio_start_read(gpio_pin_t pin, volatile struct pin_state *state, uint8_t flags);
*/

#include <kernel/list.h>
struct pcint_handler {
	struct list_head list; 
	void (*callback)(struct gpio_adapter *, void *); 
	void *data; 
}; 

LIST_HEAD(_pcint_handlers); 

static void PCINT_vect(void){
	struct pcint_handler *h = 0; 
	list_for_each_entry(h, &_pcint_handlers, list){
		if(h->callback) h->callback(atmega_gpio_get_adapter(), h->data); 	
	}
}

ISR(PCINT0_vect){
	PCINT_vect(); 
}

ISR(PCINT1_vect){
	PCINT_vect(); 
}

ISR(PCINT2_vect){
	PCINT_vect(); 
}

static void _atmega_gpio_register_pcint(struct gpio_adapter *adapter, gpio_pin_t pin, void (*handler)(struct gpio_adapter *, void *data), void *data){
	struct pcint_handler *h = 0; 
	list_for_each_entry(h, &_pcint_handlers, list){
		if(h->callback && h->callback == handler) {
			return; 		
		}
	}
	h = malloc(sizeof(struct pcint_handler)); 
	memset(h, 0, sizeof(struct pcint_handler)); 
	h->callback = handler; 
	h->data = data; 
	list_add_tail(&h->list, &_pcint_handlers); 
}

static void _atmega_gpio_enable_pcint(gpio_pin_t pin){
	((pin) >= GPIO_PB0 && (pin) <= GPIO_PB7)
		?(PCICR |= _BV(PCINT0), PCMSK0 = PCMSK0 | _BV((pin) - GPIO_PB0))
		:((pin) >= GPIO_PC0 && (pin) <= GPIO_PC7_NC)
			?(PCICR |= _BV(PCINT1), PCMSK1 = PCMSK1 | _BV((pin) - GPIO_PB0))
			:((pin) >= GPIO_PD0 && (pin) <= GPIO_PD7)
				?(PCICR |= _BV(PCINT2), PCMSK2 = PCMSK2 | _BV((pin) - GPIO_PD0))
				:(-1);
}

static void _atmega_gpio_disable_pcint(gpio_pin_t pin) {
	((pin) >= GPIO_PB0 && (pin) <= GPIO_PB7)
		?(PCICR &= ~_BV(PCINT0), PCMSK0 = PCMSK0 & ~_BV((pin) - GPIO_PB0))
		:((pin) >= GPIO_PC0 && (pin) <= GPIO_PC7_NC)
			?(PCICR &= ~_BV(PCINT1), PCMSK1 = PCMSK1 & ~ _BV((pin) - GPIO_PB0))
			:((pin) >= GPIO_PD0 && (pin) <= GPIO_PD7)
				?(PCICR &= ~_BV(PCINT2), PCMSK2 = PCMSK2 & ~ _BV((pin) - GPIO_PD0))
				:(-1); 
}

static void _atmega_gpio_configure(struct gpio_adapter *adapter, gpio_pin_t pin, uint16_t fun){
	if(fun & GP_OUTPUT) RSET(DREG(pin), PIDX(pin)); 
	else RCLR(DREG(pin), PIDX(pin)); 
	if(fun & GP_PULLUP) RSET(OREG(pin), PIDX(pin)); 
	else RCLR(OREG(pin), PIDX(pin)); 
}

static void _atmega_write_pin(gpio_pin_t pin, uint8_t val) {
	(val)?RSET(OREG(pin), PIDX(pin)):RCLR(OREG(pin), PIDX(pin)); 
}

static uint8_t _atmega_read_pin(gpio_pin_t pin) {
	return (IREG(pin) & _BV(PIDX(pin)))?1:0; 
}

static uint8_t _atmega_gpio_read_pin(struct gpio_adapter *adapter, gpio_pin_t pin){
	return _atmega_read_pin(pin); 	
}

static void _atmega_gpio_write_pin(struct gpio_adapter *adapter, gpio_pin_t pin, uint8_t value){
	_atmega_write_pin(pin, value); 	
}

static void _atmega_gpio_enable_irq(struct gpio_adapter *adapter, gpio_pin_t pin, uint8_t enabled){
	if(enabled) _atmega_gpio_enable_pcint(pin); 
	else _atmega_gpio_disable_pcint(pin); 
}

static struct gpio_adapter_ops _atmega_gpio_ops = {
	.write_pin = _atmega_gpio_write_pin, 
	.read_pin = _atmega_gpio_read_pin, 
	.configure = _atmega_gpio_configure,
	.register_irq = _atmega_gpio_register_pcint,
	.enable_irq = _atmega_gpio_enable_irq
}; 

static struct gpio_adapter _atmega_gpio = {
	.ops = &_atmega_gpio_ops 
}; 

struct gpio_adapter *atmega_gpio_get_adapter(void){
	return &_atmega_gpio; 
}
