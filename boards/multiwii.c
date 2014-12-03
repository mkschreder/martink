#include "multiwii.h"

#include <arch/soc.h>

#include <sensors/hmc5883l.h>
#include <sensors/bmp085.h>
#include <sensors/mpu6050.h>

#define FRONT_PIN PD3
#define RIGHT_PIN PB1
#define LEFT_PIN PB2
#define BACK_PIN PB3

#define RC_MAX 2000
#define RC_MIN 1000

#define PWM_UPDATE_DISABLE {TIMSK2 &= ~_BV(TOIE2);}
#define PWM_UPDATE_ENABLE {TIMSK2 |= _BV(TOIE2);}

const static uint16_t rc_defaults[6] = {1000, 1500, 1500, 1500, 1500, 1500}; 

struct board {
	volatile uint16_t pwm[6]; 
	timeout_t rc_time[6]; 
	uint16_t 	rc_value[6]; 
	volatile timeout_t ch_timeout[6]; 
	volatile timeout_t pwm_timeout; 
	timeout_t signal_timeout; 
	timeout_t last_rc_update; 
	uint16_t pwm_pulse_delay_us; 
	volatile uint8_t pwm_lock; 
}; 

static struct board _brd; 
static struct board *brd = &_brd; 
static struct uart uart;

void get_accelerometer(int16_t *x, int16_t *y, int16_t *z){
	int16_t gx, gy, gz; 
	mpu6050_getRawData(x, y, z, &gx, &gy, &gz); 
}

void get_gyroscope(float *x, float *y, float *z){
	double ax, ay, az, gx, gy, gz; 
	mpu6050_getConvData(&ax, &ay, &az, &gx, &gy, &gz);   
	*x = gx; 
	*y = gy; 
	*z = gz; 
	//mpu6050_getRawData(&ax, &ay, &az, x, y, z); 
}

void get_magnetometer(int16_t *x, int16_t *y, int16_t *z){
	double mx, my, mz; 
	hmc5883l_getdata(&mx, &my, &mz); 
	*x = mx; *y = my; *z = mz; 
}

void get_altitude(int16_t *alt){
	*alt = bmp085_getaltitude(); 
}

void get_pressure(int16_t *pres){
	*pres = bmp085_getpressure() / 10; 
}

void get_temperature(int16_t *temp){
	*temp = bmp085_gettemperature(); 
}

uint8_t get_rc_commands(int16_t *throt, int16_t *yaw, int16_t *pitch, int16_t *roll){
	return 0; 
}

void reset_rc(void){
	for(int c = 0; c < 6; c++){
		brd->rc_value[c] = rc_defaults[c]; 
	}
}

void brd_init(void){
	brd->pwm_pulse_delay_us = 10500; 
	brd->pwm_lock = 0; 
	brd->pwm_timeout = timeout_from_now(0); 
	
	// init output pins
	DDRD |= _BV(3); 
	DDRB |= _BV(1) | _BV(2) | _BV(3) | _BV(5); 
	
	// disable external ints
	EICRA = 0;
	EIMSK = 0;
	
	// enable pin interrupts
	PCICR |= _BV(PCIE2);  
	PCMSK0 = 0; //(1<<PCINT4) | (1<<PCINT5);
	PCMSK1 = 0; //(1<<PCINT8) | (1<<PCINT9) | (1<<PCINT10) | (1<<PCINT11);
	PCMSK2 |= _BV(PCINT20) | _BV(PCINT21) | _BV(PCINT22) | _BV(PCINT23) | _BV(PCINT18);
	
	uart_init(&uart, UART_BAUD_SELECT(38400, F_CPU));
	uart_printf(PSTR("booting..\n")); 
	
	time_init(); 
	
	// init all sensors
	i2c_init(); 
	hmc5883l_init(); 
	bmp085_init(); 
	mpu6050_init(); 
	
	reset_rc(); 
	// ticking timer for the pwm generator
	TCCR2B = _BV(CS22) | _BV(CS20);  // 128 prescaler
	TIMSK2 |= _BV(TOIE2); 
	TCNT2 = 0; 
	/*
	TCCR0B = _BV(CS02); //_BV(CS00); // no prescaler, 256 clocks per overflow (FCPU/256)
	TIMSK0 |= _BV(TOIE0); 
	TCNT0 = 0;*/
	
	sei(); 
	
	uart_printf(PSTR("multiwii board!\n")); 
}

void brd_process_events(void){
	//TIMSK0 |= _BV(TOIE0); 
	if(timeout_expired(brd->last_rc_update)){
		reset_rc(); 
	}
	//_interrupt_pwm_update(); 
	/*if(timeout_expired(brd->signal_timeout)){
		for(int c = 0; c < 6; c++)
			set_pin(PWM_FRONT + c, 0); 
	}*/
}

static volatile uint8_t ch = 0; 
	
ISR(TIMER2_OVF_vect)
{ 
	sei(); 
	PWM_UPDATE_DISABLE; 
	
	switch(PWM_FRONT + ch){
		case PWM_FRONT: PORTD |= _BV(3); break; 
		case PWM_BACK: PORTB |= _BV(1); break; 
		case PWM_RIGHT: PORTB |= _BV(2); break; 
		case PWM_LEFT: PORTB |= _BV(3); break; 
	}
	timeout_t tout = timeout_from_now(brd->pwm[ch]); 
	//int16_t d = brd->pwm[ch]; 
	while(!timeout_expired(tout)); // _delay_us(1); 
	switch(PWM_FRONT + ch){
		case PWM_FRONT: PORTD &= ~_BV(3); break; 
		case PWM_BACK: PORTB 	&= ~_BV(1); break; 
		case PWM_RIGHT: PORTB &= ~_BV(2); break; 
		case PWM_LEFT: PORTB 	&= ~_BV(3); break; 
	}
	ch++; 
	if(ch == 6) ch = 0; 
	PWM_UPDATE_ENABLE; 
	TIFR2 |= _BV(TOV2);
	/*
	if(timeout_expired(brd->pwm_timeout)){
		brd->pwm_timeout = timeout_from_now(brd->pwm_pulse_delay_us); 
		
		for(int c = 0; c < 6; c++){
			brd->ch_timeout[c] = timeout_from_now(brd->pwm[c]); 
			switch(PWM_FRONT + c){
				case PWM_FRONT: PORTD |= _BV(3); break; 
				case PWM_LEFT: PORTB |= _BV(1); break; 
				case PWM_RIGHT: PORTB |= _BV(2); break; 
				case PWM_BACK: PORTB |= _BV(3); break; 
			}
		}
	}
	for(int c = 0; c < 6; c++){
		if(timeout_expired(brd->ch_timeout[c])){
			switch(PWM_FRONT + c){
				case PWM_FRONT: PORTD &= ~_BV(3); break; 
				case PWM_LEFT: PORTB 	&= ~_BV(1); break; 
				case PWM_RIGHT: PORTB &= ~_BV(2); break; 
				case PWM_BACK: PORTB 	&= ~_BV(3); break; 
			}
		} 
	}*/
	/*TCNT0 = 0; 
	TIFR0 |= _BV(TOV0); */
}

void set_pin(uint8_t pin, uint16_t value){
	if(pin == LED_PIN) {
		if(value) PORTB |= _BV(5); 
		else PORTB &= ~_BV(5); 
	} else if(pin >= PWM_FRONT && pin < PWM_COUNT){
		if(value > PWM_MAX) value = PWM_MAX; 
		if(value < PWM_MIN) value = PWM_MIN; 
		
		//PWM_UPDATE_DISABLE; 
		brd->pwm[pin - PWM_FRONT] = value; 
		//PWM_UPDATE_ENABLE; 
	}
	/*if(value == 1){
		switch(pin){
			case PWM_FRONT: PORTD |= _BV(3); break; 
			case PWM_LEFT: PORTB |= _BV(1); break; 
			case PWM_RIGHT: PORTB |= _BV(2); break; 
			case PWM_BACK: PORTB |= _BV(3); break; 
			case LED_PIN: PORTB |= _BV(5); break; 
		}
	} else if(value == 0){
		switch(pin){
			case PWM_FRONT: PORTD &= ~_BV(3); break; 
			case PWM_LEFT: PORTB &= ~_BV(1); break; 
			case PWM_RIGHT: PORTB &= ~_BV(2); break; 
			case PWM_BACK: PORTB &= ~_BV(3); break; 
			case LED_PIN: PORTB &= ~_BV(5); break; 
		}
	}*/
	/*switch(c){
		case 0: PORTD |= _BV(3); break; 
		case 1: PORTB |= _BV(1); break; 
		case 2: PORTB |= _BV(2); break; 
		case 3: PORTB |= _BV(3); break; 
	}
	if(pin >= PWM_FRONT && pin < PWM_COUNT){
		uint8_t idx = pin - PWM_FRONT; 
		if(idx > 5) return; 
		brd->pwm[idx] = value; 
		brd->signal_timeout = timeout_from_now(1000000); 
	}*/
}

uint16_t get_pin(uint8_t pin){
	if(pin >= RC_IN0 && pin <= RC_IN4){
		uint16_t val = brd->rc_value[pin - RC_IN0]; 
		if(val > RC_MAX) return RC_MAX; 
		if(val < RC_MIN) return RC_MIN; 
		return val; 
	}
	return 0; 
}

void _compute_rc_time(uint8_t chan, uint8_t val, timeout_t time){
	// if the pin went high then reset timer
	if(val){
		brd->rc_time[chan] = time; 
	} else {
		// if it went low then measure microseconds and save the value
		timeout_t dv = time_clock_to_us(time - brd->rc_time[chan]);
		if(dv > RC_MAX) 
			brd->rc_value[chan] = RC_MAX; 
		else if(dv < RC_MIN) 
			brd->rc_value[chan] = RC_MIN; 
		else 
			brd->rc_value[chan] = dv; 
	}
}
/*
/// triggered for channel 5 and 6 
ISR(PCINT0_vect){
	static volatile uint8_t prev_pinb = 0xff;
	
	timeout_t time = time_get_clock();
	
  register uint8_t pinb = PINB & 0xff;
	
	register uint8_t changed = (pinb ^ prev_pinb);
	prev_pinb = pinb;

	for(uint8_t c = 1; c < 4; c++){
		if(changed & _BV(c)){
			_compute_rc_time(c, pinb & _BV(c), time);
		} 
	}
}*/

ISR(PCINT2_vect){
	static volatile uint8_t prev_pind = 0xFF;

	uint8_t pind = PIND & 0xff;
	
	uint8_t changed = (pind ^ prev_pind);
	prev_pind = pind;

	timeout_t time = time_get_clock();
	
	//if receiver is just powered on 
	if(timeout_expired(brd->last_rc_update)){
		for(int c = 0; c < 6; c++){
			brd->rc_time[c] = timeout_from_now(0); 
		}
		brd->last_rc_update = timeout_from_now(1000000); 
	} else {
		brd->last_rc_update = timeout_from_now(1000000); 
		
		uint8_t pins[] = {2, 4, 5, 6, 7}; 
		for(uint8_t c = 0; c < sizeof(pins); c++){
			if(changed & _BV(pins[c])){
				_compute_rc_time(c, pind & _BV(pins[c]), time); 
			}
		}
	}
}
