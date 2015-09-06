#include <arch/soc.h>
#include "fst6_keys.h"

#define KEYS_COL1 GPIO_PB8
#define KEYS_COL2 GPIO_PB9
#define KEYS_COL3 GPIO_PB10
#define KEYS_COL4 GPIO_PB11
#define KEYS_ROW1 GPIO_PB12
#define KEYS_ROW2 GPIO_PB13
#define KEYS_ROW3 GPIO_PB14
#define KEYS_ROTA GPIO_PC14
#define KEYS_ROTB GPIO_PC15
#define KEYS_SWA GPIO_PB0
#define KEYS_SWB GPIO_PB1
#define KEYS_SWC GPIO_PB5
#define KEYS_SWD GPIO_PC13

/*
static const gpio_pin_t _columns[] = {KEYS_COL1, KEYS_COL2, KEYS_COL3, KEYS_COL4}; 
static const gpio_pin_t _rows[] = {KEYS_ROW1, KEYS_ROW2, KEYS_ROW3}; 
*/

static uint32_t _fst6_keys_scan(void){
	uint32_t keys = 0; 
	
	gpio_set(KEYS_COL1); 
	gpio_set(KEYS_COL2); 
	gpio_set(KEYS_COL3); 
	gpio_set(KEYS_COL4); 
	gpio_clear(KEYS_COL1); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= (1 << FST6_KEY_CH1P); 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= (1 << FST6_KEY_CH3P); 
	gpio_set(KEYS_COL1); 
	gpio_clear(KEYS_COL2); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= (1 << FST6_KEY_CH1M); 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= (1 << FST6_KEY_CH3M); 
	if(!gpio_read_pin(KEYS_ROW3)) keys |= (1 << FST6_KEY_SELECT); 
	gpio_set(KEYS_COL2); 
	gpio_clear(KEYS_COL3); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= (1 << FST6_KEY_CH2P); 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= (1 << FST6_KEY_CH4P); 
	if(!gpio_read_pin(KEYS_ROW3)) keys |= (1 << FST6_KEY_OK); 
	gpio_set(KEYS_COL3); 
	gpio_clear(KEYS_COL4); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= (1 << FST6_KEY_CH2M); 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= (1 << FST6_KEY_CH4M); 
	if(!gpio_read_pin(KEYS_ROW3)) keys |= (1 << FST6_KEY_CANCEL); 
	gpio_set(KEYS_COL4);
	//gpio_clear(KEYS_COL1); 
	//gpio_clear(KEYS_COL2); 
	//gpio_clear(KEYS_COL3); 
	//gpio_clear(KEYS_COL4); 
	
	//if(!gpio_read_pin(KEYS_ROTA)) keys |= (1 << FST6_KEY_ROTA); 
	//if(!gpio_read_pin(KEYS_ROTB)) keys |= (1 << FST6_KEY_ROTB); 
	if(!gpio_read_pin(KEYS_SWA)) keys |= (1 << FST6_KEY_SWA); 
	if(!gpio_read_pin(KEYS_SWB)) keys |= (1 << FST6_KEY_SWB); 
	if(!gpio_read_pin(KEYS_SWC)) keys |= (1 << FST6_KEY_SWC); 
	if(!gpio_read_pin(KEYS_SWD)) keys |= (1 << FST6_KEY_SWD); 
	
	return keys; 
}

static void _fst6_rotary_irq(struct irq *irq, uint32_t mask){
	struct fst6_keys *self = container_of(irq, struct fst6_keys, irq); 
	
	const uint32_t our_pins = 
		GPIO_PIN_BIT(KEYS_ROTA) | 
		GPIO_PIN_BIT(KEYS_ROTB); 

	static timestamp_t _timeout = 0, _timeout2 = 0; 
	static uint8_t _forward = 0; 
	if(!(mask & our_pins)) return; 
	
	if(mask & GPIO_PIN_BIT(KEYS_ROTA)){
		if(timestamp_expired(_timeout)){
			_forward = 1; 
		}
	} else if(mask & GPIO_PIN_BIT(KEYS_ROTB)){
		if(timestamp_expired(_timeout)){
			_forward = 0; 
		}
	}
	if(_forward && timestamp_expired(_timeout2)){
		cbuf_put(&self->buffer, FST6_KEY_ROTA); 
		cbuf_put(&self->buffer, FST6_KEY_FLAG_UP | FST6_KEY_ROTA); 
		_timeout2 = timestamp_from_now_us(100000); 
	} else if(timestamp_expired(_timeout2)){
		cbuf_put(&self->buffer, FST6_KEY_ROTB); 
		cbuf_put(&self->buffer, FST6_KEY_FLAG_UP | FST6_KEY_ROTB); 
		_timeout2 = timestamp_from_now_us(100000); 
	}
	// the timeout needs to be long enough, but if it is too long then user 
	// will experience that if he rapidly changes direction, the driver
	// will continue reporting the previous direction until this timeout
	// expires. 
	_timeout = timestamp_from_now_us(50000); 
}

static PT_THREAD(_fst6_keys_thread(struct libk_thread *kthread, struct pt *pt)){
	struct fst6_keys *self = container_of(kthread, struct fst6_keys, thread); 
	
	PT_BEGIN(pt); 
	
	while(1){
		uint32_t keys = _fst6_keys_scan(); 
		uint32_t changed = keys ^ self->pressed; 
		for(int c = 0; c < FST6_KEYS_COUNT; c++){
			// if a key is pressed down 
			if((changed & (1 << c)) && (keys & (1 << c))){
				cbuf_put(&self->buffer, c); 
			} else if((changed & (1 << c)) && !(keys & (1 << c))){
				cbuf_put(&self->buffer, FST6_KEY_FLAG_UP | c); 
			}
		}
		self->pressed = keys; 
		PT_YIELD(pt); 
	}
	
	PT_END(pt); 
}

void fst6_keys_init(struct fst6_keys *self){
	cbuf_init(&self->buffer, self->buffer_data, 8); 
	
	gpio_configure(KEYS_ROW1, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROW2, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROW3, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROTA, GP_INPUT | GP_PULLUP | GP_PCINT); 
	gpio_configure(KEYS_ROTB, GP_INPUT | GP_PULLUP | GP_PCINT); 
	gpio_configure(KEYS_SWA, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWB, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWC, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWD, GP_INPUT | GP_PULLUP); 
	
	gpio_configure(KEYS_COL1, GP_OUTPUT); 
	gpio_configure(KEYS_COL2, GP_OUTPUT); 
	gpio_configure(KEYS_COL3, GP_OUTPUT); 
	gpio_configure(KEYS_COL4, GP_OUTPUT); 
	
	gpio_clear(KEYS_COL1); 
	gpio_clear(KEYS_COL2); 
	gpio_clear(KEYS_COL3); 
	gpio_clear(KEYS_COL4); 
	
	gpio_register_irq(&self->irq, _fst6_rotary_irq); 
	
	libk_create_thread(&self->thread, _fst6_keys_thread, "fst6_kb"); 
}

int16_t fst6_keys_get(struct fst6_keys *self){
	if(cbuf_get_waiting(&self->buffer) == 0) return -1; 
	return cbuf_get(&self->buffer); 
}

uint8_t fst6_keys_key_down(struct fst6_keys *self, fst6_key_code_t key){
	if(key >= 32) return 0; 
	return (self->pressed & (1 << key))?1:0; 
}
