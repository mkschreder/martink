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
#include <radio/nrf24l01.h>
#include <disp/ssd1306.h>

#include <math.h>


#define FRONT_PIN PD3
#define RIGHT_PIN PB1
#define LEFT_PIN PB2
#define BACK_PIN PB3

#define RC_MAX 2000
#define RC_MIN 1000

#define PWM_UPDATE_DISABLE {TIMSK2 &= ~_BV(TOIE2);}
#define PWM_UPDATE_ENABLE {TIMSK2 |= _BV(TOIE2);}

#define ACCNAME mpu6050

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

void get_accelerometer(float *x, float *y, float *z){
	
}

void get_gyroscope(float *x, float *y, float *z){
	
}

void get_magnetometer(int16_t *x, int16_t *y, int16_t *z){
	
}

void get_altitude(int16_t *alt){
	
}

void get_pressure(int16_t *pres){
	
}

void get_temperature(int16_t *temp){
	
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
	/* The general init (clock, libc, watchdog disable) */
  cpu_init();
  
  uart0_init(38400); 
  uart0_printf("Foo %d\n", 10); 
	
	pmc_enable_periph_clk(ID_PIOA); 
	pmc_enable_periph_clk(ID_PIOB); 
	pmc_enable_periph_clk(ID_PIOC); 
	pmc_enable_periph_clk(ID_PIOD); 
  pmc_enable_periph_clk(ID_USART0);
  pmc_enable_periph_clk(ID_USART1);
  
/*
	NVIC_DisableIRQ(USART0_IRQn);
	NVIC_ClearPendingIRQ(USART0_IRQn);
	NVIC_SetPriority(USART0_IRQn,5);
	NVIC_EnableIRQ(USART0_IRQn);*/

  /* Board pin 13 == PB27 */
  PIO_Configure(PIOB, PIO_OUTPUT_1, PIO_PB27, PIO_DEFAULT);
  
  PIO_Configure(PIOA, PIO_PERIPH_A, PIO_PA13A_TXD1, PIO_DEFAULT);
  PIO_Configure(PIOA, PIO_PERIPH_A, PIO_PA12A_RXD1, PIO_DEFAULT);
  
	// Disable PDC channel
	//USART1->US_PTCR = US_PTCR_RXTDIS | US_PTCR_TXTDIS ;

	// Reset and disable receiver and transmitter
	USART1->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS ;

	// Configure mode
	USART1->US_MR = USART_MODE_ASYNCHRONOUS;


	// Configure baudrate, asynchronous no oversampling
	USART1->US_BRGR = (SystemCoreClock / 38400) / 16 ;

	// Configure interrupts
	USART1->US_IDR = 0xFFFFFFFF;
	USART1->US_IER = US_IER_RXRDY | US_IER_OVRE | US_IER_FRAME;

	// Enable UART interrupt in NVIC
	NVIC_EnableIRQ( USART1_IRQn ) ;

	// Enable receiver and transmitter
	USART1->US_CR = US_CR_RXEN | US_CR_TXEN ;

	i2c_init(); 
	spi0_init(); 
	
	struct nrf24l01 nrf1 = {
		.spi = SPI_API(spi0), 
		.cs_pin = GPIO_PA15, 
		.ce_pin = GPIO_PD1
	}; 
	nrf24l01_init(&nrf1); 
	
	gpio_set_function(GPIO_PB26, GP_OUTPUT); 
	gpio_set_function(GPIO_PA16, GP_OUTPUT); 
	
  while(1) {
		char buf[NRF24L01_PAYLOAD]; 
		nrf24l01_write(&nrf1, buf); 
		
    time_delay(250000L);
    
    i2c_start_wait(0x11 | I2C_WRITE);
		i2c_write(0x40);
		i2c_stop();
		
		//spi0_writereadbyte(0x11); 
		
		uart0_printf("Bar %d\n", 10); 
  
    USART_PutChar(USART1, 'D'); 
    //USART_PutChar(USART0, 'D'); 
    
    if(gpio_read(GPIO_PB27)){
			gpio_clear(GPIO_PB27); 
		} else {
			gpio_set(GPIO_PB27); 
		}
  }
}

void brd_process_events(void){
	
}

static volatile uint8_t ch = 0; 


void set_pin(uint8_t pin, uint16_t value){
	if(pin == LED_PIN) {
		if(value) PIOB->PIO_SODR = PIO_PB27; 
		else PIOB->PIO_CODR = PIO_PB27; 
	}
}

uint16_t get_pin(uint8_t pin){
	if(pin >= RC_IN0 && pin <= RC_IN4){
		uint16_t val;
		val = brd->rc_value[pin - RC_IN0]; 
		if(val > RC_MAX) return RC_MAX; 
		if(val < RC_MIN) return RC_MIN; 
		return val; 
	}
	return 0; 
}
