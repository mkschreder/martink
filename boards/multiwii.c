/**
	This file is part of martink project.

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

#include "multiwii.h"

#include <arch/soc.h>
#include "../util.h"

#include <sensors/hmc5883l.h>
#include <sensors/bmp085.h>
#include <sensors/mpu6050.h>
#include <disp/ssd1306.h>
#include <util/serial_debugger.h>

#include <radio/nrf24l01.h>

#include <math.h>
#include <string.h>

#define FRONT_PIN PD3
#define RIGHT_PIN PB1
#define LEFT_PIN PB2
#define BACK_PIN PB3

#define RC_MAX 2000
#define RC_MIN 1000

#define PWM_UPDATE_DISABLE {TIMSK2 &= ~_BV(TOIE2);}
#define PWM_UPDATE_ENABLE {TIMSK2 |= _BV(TOIE2);}

/*
#define PREFIX foo
#define CALL2(P,x) _##P##_bar(x)
#define CALL(P, x) CALL2(P,x)
*/

//#define acc_read(ACC, x, y, z) _##ACC_getConvAcc(x, y, z); 
//#define gyr_read(x, y, z) _ACCNAME##_getConvGyr(x, y, z); 

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
	struct bmp085 bmp;
	struct mpu6050 mpu; 
	struct twi_device twi0; 
	struct ssd1306 ssd; 
}; 

static struct board _brd; 
static struct board *brd = &_brd;

void get_accelerometer(float *x, float *y, float *z){
	double ax, ay, az; 
	//CALL(foo, 10); 
	mpu6050_getConvAcc(&brd->mpu, &ax, &ay, &az); 
	*x = ax; 
	*y = ay; 
	*z = az; 
}

void get_gyroscope(float *x, float *y, float *z){
	double gx, gy, gz; 
	mpu6050_getConvGyr(&brd->mpu, &gx, &gy, &gz);   
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
	*alt = bmp085_getaltitude(&brd->bmp); 
}

void get_pressure(int16_t *pres){
	*pres = bmp085_getpressure(&brd->bmp) / 10; 
}

void get_temperature(int16_t *temp){
	*temp = bmp085_gettemperature(&brd->bmp); 
}

uint8_t get_rc_commands(int16_t *throt, int16_t *yaw, int16_t *pitch, int16_t *roll){
	return 0; 
}

void reset_rc(void){
	for(int c = 0; c < 6; c++){
		brd->rc_value[c] = rc_defaults[c]; 
	}
}

#define GPIO_MWII_MOTOR0 	GPIO_PD3
#define GPIO_MWII_MOTOR1 	GPIO_PD5
#define GPIO_MWII_MOTOR2 	GPIO_PD6
#define GPIO_MWII_MOTOR3 	GPIO_PB3

#define GPIO_MWII_RX0			GPIO_PD2
#define GPIO_MWII_RX1			GPIO_PD4
#define GPIO_MWII_RX2			GPIO_PD7
#define GPIO_MWII_RX3			GPIO_PB0

#define GPIO_MWII_LED			GPIO_PB5

#define GPIO_RC0 GPIO_PD2
#define GPIO_RC1 GPIO_PD4
#define GPIO_RC2 GPIO_PD7
#define GPIO_RC3 GPIO_PB0

void compute_rc_values(void){
	timeout_t t_up, t_down; 
	#define abs(x) ((x >= 0)?x:-x)
	
	#define COMPUTE_RC_CHAN(ch) {\
		timeout_t ticks = time_clock_to_us(t_down - t_up);\
		if(abs(brd->rc_value[ch] - ticks) > 10 && ticks > RC_MIN && ticks < RC_MAX)\
			brd->rc_value[ch] = ticks;\
	}
	
	if(gpio_get_status(GPIO_RC0, &t_up, &t_down) & GP_WENT_LOW){
		COMPUTE_RC_CHAN(0); 
	}
	if(gpio_get_status(GPIO_RC1, &t_up, &t_down) & GP_WENT_LOW){
		COMPUTE_RC_CHAN(1); 
	}
	if(gpio_get_status(GPIO_RC2, &t_up, &t_down) & GP_WENT_LOW){
		COMPUTE_RC_CHAN(2); 
	}
	if(gpio_get_status(GPIO_RC3, &t_up, &t_down) & GP_WENT_LOW){
		COMPUTE_RC_CHAN(3); 
	}
}

void brd_init(void){
	
	brd->pwm_pulse_delay_us = 10500; 
	brd->pwm_lock = 0; 
	brd->pwm_timeout = timeout_from_now(0); 

	gpio_configure(GPIO_MWII_LED, GP_OUTPUT); 
	//gpio_set(GPIO_MWII_LED); 
	
	gpio_configure(GPIO_MWII_MOTOR0, GP_OUTPUT);
	gpio_configure(GPIO_MWII_MOTOR1, GP_OUTPUT);
	gpio_configure(GPIO_MWII_MOTOR2, GP_OUTPUT);
	gpio_configure(GPIO_MWII_MOTOR3, GP_OUTPUT);
	
	gpio_configure(GPIO_MWII_RX0, GP_INPUT | GP_PULLUP);
	gpio_configure(GPIO_MWII_RX1, GP_INPUT | GP_PULLUP);
	gpio_configure(GPIO_MWII_RX2, GP_INPUT | GP_PULLUP);
	gpio_configure(GPIO_MWII_RX3, GP_INPUT | GP_PULLUP);
	
	// disable external ints
	EICRA = 0;
	EIMSK = 0;
	
	// enable pin interrupts
	PCICR |= _BV(PCIE0) | _BV(PCIE2);  
	PCMSK0 = _BV(PCINT0); // | _BV(PCINT2);
	PCMSK1 = 0; //(1<<PCINT8) | (1<<PCINT9) | (1<<PCINT10) | (1<<PCINT11);
	PCMSK2 |=  _BV(PCINT18) | _BV(PCINT20) /*| _BV(PCINT21) | _BV(PCINT22) */| _BV(PCINT23);
	
	sei(); 
	
	time_init(); 
	uart0_init(38400);
	uart0_puts("booting..\n"); 
	
	twi0_init_default(); 
	spi0_init(); 

	if(!twi_get_interface(0, &brd->twi0)){
		uart0_puts("NO I2C!\n"); 
		while(1); 
	}
	/*
	struct nrf24l01 nrf; 
	struct serial_debugger debug; 
	struct serial_interface spi = SPI_DEVICE_INTERFACE(spi0); 
	struct serial_interface console = UART_DEVICE_INTERFACE(uart0); 
	serial_debugger_init(&debug, &spi, &console); 
	
	nrf24l01_init(&nrf, &debug.interface, GPIO_PB0, GPIO_PB1); 
	while(1){
		unsigned char data[32] = {0}; 
		strcpy(data, "Hello World!"); 
		nrf24l01_write(&nrf, data); 
		while(1); 
	}*/
	// init all sensors
	
	
	//hmc5883l_init(); 
	uart0_puts("INIT MPU..\n"); _delay_ms(200); 
	mpu6050_init(&brd->mpu, &brd->twi0.interface); 
	uart0_printf("MPU6050 .. %s\n", ((mpu6050_probe(&brd->mpu))?"found":"not found!")); 
	
	uart0_printf("INIT BMP (%04x)..\n", brd->twi0.interface.write); _delay_ms(200); 
	bmp085_init(&brd->bmp, &brd->twi0.interface); 
	
	reset_rc(); 
	// ticking timer for the pwm generator
	/*TCCR2B = _BV(CS22) | _BV(CS20);  // 128 prescaler
	TIMSK2 |= _BV(TOIE2); 
	TCNT2 = 0; 
	*/
	TCCR0A = _BV(COM0A1) | _BV(COM0B1) | _BV(WGM01) | _BV(WGM00); 
	TCCR0B = _BV(CS02); 
	TCNT0 = 0; 
	OCR0A = map(MINCOMMAND, 0, PWM_MAX, 0, 127); 
	OCR0B = map(MINCOMMAND, 0, PWM_MAX, 0, 127); 
	
	TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); 
	TCCR2B = _BV(CS22) | _BV(CS20); 
	TCNT2 = 0; 
	OCR2A = map(MINCOMMAND, 0, PWM_MAX, 0, 250); 
	OCR2B = map(MINCOMMAND, 0, PWM_MAX, 0, 250); ; 
	
	/*
	TCCR0B = _BV(CS02); //_BV(CS00); // no prescaler, 256 clocks per overflow (FCPU/256)
	TIMSK0 |= _BV(TOIE0); 
	TCNT0 = 0;*/
	
	uint16_t pres = 0; get_pressure(&pres); 
	int16_t temp = 0; get_temperature(&temp); 
	uart0_printf("Pressure: %d\n", pres); 
	uart0_printf("Temperature: %d\n", temp); 
	
	uart0_puts("multiwii board!\n"); 
}

void brd_process_events(void){
	compute_rc_values(); 
	
	//TIMSK0 |= _BV(TOIE0); 
	if(timeout_expired(brd->last_rc_update)){
		reset_rc(); 
	}
	
	// rc reset routine
	if(timeout_expired(brd->last_rc_update)){
		for(int c = 0; c < 6; c++){
			brd->rc_time[c] = timeout_from_now(0); 
		}
		brd->last_rc_update = timeout_from_now(1000000); 
	} 
}

static volatile uint8_t ch = 0; 
/*
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
	
}*/

void set_pin(uint8_t pin, uint16_t value){
	if(pin == LED_PIN) {
		if(value) PORTB |= _BV(5); 
		else PORTB &= ~_BV(5); 
	} else if(pin == PWM_LEFT){
		OCR2A = map(value, 0, PWM_MAX, 0, 250); 
	} else if(pin == PWM_RIGHT){
		OCR2B = map(value, 0, PWM_MAX, 0, 250); 
	} else if(pin == PWM_FRONT){
		OCR0A = map(value, 0, PWM_MAX, 0, 127); 
	} else if(pin == PWM_BACK){
		OCR0B = map(value, 0, PWM_MAX, 0, 127); 
	}
	/*else if(pin >= PWM_FRONT && pin < PWM_COUNT){
		if(value > PWM_MAX) value = PWM_MAX; 
		if(value < PWM_MIN) value = PWM_MIN; 
		
		//PWM_UPDATE_DISABLE; 
		//brd->pwm[pin - PWM_FRONT] = value; 
		//PWM_UPDATE_ENABLE; 
	}*/
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
		uint16_t val;
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
			val = brd->rc_value[pin - RC_IN0]; 
		}
		if(val > RC_MAX) return RC_MAX; 
		if(val < RC_MIN) return RC_MIN; 
		return val; 
	}
	return 0; 
}

/*
ISR(PCINT0_vect){
	timeout_t time = time_get_clock();
	static volatile uint8_t prev = 0;
	uint8_t value = (PINB & (_BV(0))) | (PIND & (_BV(2) | _BV(4) | _BV(7))); 
	uint8_t changed = (value ^ prev);
	prev = value;
	
	brd->last_rc_update = time + 1000000; 
	
	#define COMPUTE_RC_CHAN(ch) {\
		if(set){ brd->rc_time[ch] = time; } \
		else { \
			timeout_t ticks = time_clock_to_us(time - brd->rc_time[ch]);\
			if(abs(brd->rc_value[ch] - ticks) > 10 && ticks > RC_MIN && ticks < RC_MAX)\
				brd->rc_value[ch] = ticks;\
		}\
	}
	
	#define abs(x) ((x >= 0)?x:-x)
	for(uint8_t c = 0; c < 8; c++){
		if(!(changed & _BV(c))) continue; 
		uint8_t set = value & _BV(c); 
		switch(c){
			case 2: 
				COMPUTE_RC_CHAN(0);
				break; 
			case 4: 
				COMPUTE_RC_CHAN(1);
				break; 
			case 7: 
				COMPUTE_RC_CHAN(2);
				break; 
			case 0: 
				COMPUTE_RC_CHAN(3);
				break; 
		}
	}
}

ISR(PCINT2_vect, ISR_ALIASOF(PCINT0_vect));*/
