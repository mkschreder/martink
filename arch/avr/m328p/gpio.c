struct pin_decl {
	uint8_t pin_id; 
	uint8_t reg_bit; 
	uint8_t *out_reg;
	uint8_t *in_reg;
	uint8_t	*ddr_reg;
}; 

static struct pin_decl gPinDecl _pins[] PROGMEM = {
	{GPIO_NONE, 0, 0, 0, 0}, 
	/*GPIO_A0,
	GPIO_A1,
	GPIO_A2,
	GPIO_A3,
	GPIO_A4,
	GPIO_A5,
	GPIO_A6,
	GPIO_A7,*/
	{GPIO_B0, 0, &PORTB, &PINB, &DDRB},
	{GPIO_B1, 1, &PORTB, &PINB, &DDRB},
	{GPIO_B2, 2, &PORTB, &PINB, &DDRB},
	{GPIO_B3, 3, &PORTB, &PINB, &DDRB},
	{GPIO_B4, 4, &PORTB, &PINB, &DDRB},
	{GPIO_B5, 5, &PORTB, &PINB, &DDRB},
	{GPIO_B6, 6, &PORTB, &PINB, &DDRB},
	{GPIO_B7, 7, &PORTB, &PINB, &DDRB},
	{GPIO_C0, 0, &PORTC, &PINC, &DDRC},
	{GPIO_C1, 1, &PORTC, &PINC, &DDRC},
	{GPIO_C2, 2, &PORTC, &PINC, &DDRC},
	{GPIO_C3, 3, &PORTC, &PINC, &DDRC},
	{GPIO_C4, 4, &PORTC, &PINC, &DDRC},
	{GPIO_C5, 5, &PORTC, &PINC, &DDRC},
	{GPIO_C6, 6, &PORTC, &PINC, &DDRC},
	//GPIO_C7,
	{GPIO_D0, 0, &PORTD, &PIND, &DDRD},
	{GPIO_D1, 1, &PORTD, &PIND, &DDRD},
	{GPIO_D2, 2, &PORTD, &PIND, &DDRD},
	{GPIO_D3, 3, &PORTD, &PIND, &DDRD},
	{GPIO_D4, 4, &PORTD, &PIND, &DDRD},
	{GPIO_D5, 5, &PORTD, &PIND, &DDRD},
	{GPIO_D6, 6, &PORTD, &PIND, &DDRD},
	{GPIO_D7, 7, &PORTD, &PIND, &DDRD},
};

#define OUT_REG(pin) 	*((uint8_t*)gpm_read_byte(&gPinDecl[pin].out_reg))
#define IN_REG(pin) 	*((uint8_t*)gpm_read_byte(&gPinDecl[pin].in_reg))
#define DDR_REG(pin) 	*((uint8_t*)gpm_read_byte(&gPinDecl[pin].ddr_reg))
#define PIN_BIT(pin) 	 ((uint8_t*)gpm_read_byte(&gPinDecl[pin].reg_bit))

void gpio_write_pin(gpio_pin_t pin, uint8_t value) {
	if(value)
		OUT_REG(pin) |= _BV(PIN_BIT(pin));
	else
		OUT_REG(pin) &= ~_BV(PIN_BIT(pin)); 
}

uint8_t gpio_read_pin(gpio_pin_t pin){
	return (IN_REG(pin) & _BV(PIN_BIT(pin)))?1:0;
}

void gpio_set_function(gpio_pin_t pin, uint8_t fun){
	if(fun == GP_OUTPUT){
		DDR_REG(pin) |= _BV(PIN_BIT(pin));
	} else if(fun == GP_INPUT){
		DDR_REG(pin) &= ~_BV(PIN_BIT(pin));
	}
}

void gpio_set_pullup(gpio_pin_t pin, uint8_t pullup){
	gpio_set_function(pin, GP_INPUT); 
	gpio_write_pin(pin, 1);
}
