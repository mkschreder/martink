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

////// THIS IS ONLY A TEST FILE FOR TESTING //////

#include "multiwii.h"

#include <arch/soc.h>
#include "../util.h"

#include "interface.h"

#include <sensors/hmc5883l.h>
#include <sensors/bmp085.h>
#include <sensors/mpu6050.h>
#include <radio/nrf24l01.h>
#include <disp/ssd1306.h>

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

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
	uint16_t 				rc_values[6]; 
	timestamp_t 		rc_reset_timeout; 
	struct parallel_interface gpio0; 
	struct bmp085 bmp;
	struct mpu6050 mpu;
	struct hmc5883l hmc; 
	struct twi_device twi0; 
	struct ssd1306 ssd;
	struct fc_quad_interface interface; 
}; 

static struct board _brd; 
static struct board *brd = &_brd; 

void _due_read_accelerometer(struct fc_quad_interface *self, float *x, float *y, float *z){
	
}

void _due_read_gyroscope(struct fc_quad_interface *self, float *x, float *y, float *z){
	
}

void _due_read_magnetometer(struct fc_quad_interface *self, float *x, float *y, float *z){
	
}

int16_t _due_read_altitude(struct fc_quad_interface *self){
	
}

int16_t _due_read_pressure(struct fc_quad_interface *self){
	
}

int16_t _due_read_temperature(struct fc_quad_interface *self){
	
}


void _due_write_motors(struct fc_quad_interface *self,
	uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	/*pwm0_set(front);
	pwm1_set(back);
	pwm4_set(left);
	pwm5_set(right); */
}

uint8_t _due_read_receiver(struct fc_quad_interface *self, 
		uint16_t *rc_thr, uint16_t *rc_pitch, uint16_t *rc_yaw, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1) {
	*rc_thr = 		brd->rc_values[RC_THROTTLE]; 
	*rc_pitch = 	brd->rc_values[RC_PITCH]; 
	*rc_yaw = 		brd->rc_values[RC_YAW]; 
	*rc_roll = 		brd->rc_values[RC_ROLL]; 
	*rc_aux0 = 		brd->rc_values[RC_MODE]; 
	*rc_aux1 = 		brd->rc_values[RC_MODE2];
	
	// prevent small changes when stick is not touched
	if(abs(*rc_pitch - 1500) < 20) *rc_pitch = 1500; 
	if(abs(*rc_roll - 1500) < 20) *rc_roll = 1500; 
	if(abs(*rc_yaw - 1500) < 20) *rc_yaw = 1500;

	return 0; 
}

void reset_rc(void){
	for(int c = 0; c < 6; c++){
		brd->rc_values[c] = rc_defaults[c]; 
	}
}

void due_init(void){
	/* The general init (clock, libc, watchdog disable) */
  cpu_init();
 
  //soc_init(&cpu); 
  
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
	NVIC_EnableIRQ(USART0_IRQn);

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
*/
	twi0_init_default(); 
	spi0_init(); 
	
	//struct nrf24l01 nrf1;  
	//nrf24l01_init(&nrf1, &cpu.spi0, GPIO_PA15, GPIO_PD1); 
	
	gpio_configure(GPIO_PB26, GP_OUTPUT); 
	gpio_configure(GPIO_PA16, GP_OUTPUT); 
	
  while(1) {
		uint8_t buf[NRF24L01_PAYLOAD]; 
		//nrf24l01_write(&nrf1, buf); 
		
    delay_us(250000L);

    uint8_t data = 0x11; 
    twi0_start_write(0x11, &data, 1);
		twi0_stop();
		
		//spi0_writereadbyte(0x11); 
		
		uart0_printf("Bar %d\r\n", 10); 
		///struct d_char *uart = &cpu.uart0; 
		//uart->putc('D'); 
		//cpu.uart0.putc('D'); 
		//cpu.uart0.write("Test\n", 5); 
		
    USART_PutChar(USART1, 'D'); 
    //USART_PutChar(USART0, 'D'); 
    
    if(gpio_read_pin(GPIO_PB27)){
			gpio_clear(GPIO_PB27); 
		} else {
			gpio_set(GPIO_PB27); 
		}
  }
}

void due_process_events(void){
	
}


struct fc_quad_interface due_get_fc_quad_interface(void){
	return (struct fc_quad_interface){
		.read_accelerometer = _due_read_accelerometer,
		.read_gyroscope = _due_read_gyroscope,
		.read_magnetometer = _due_read_magnetometer,
		.read_pressure = _due_read_pressure,
		.read_altitude = _due_read_altitude,
		.read_temperature = _due_read_temperature, 
		.read_receiver = _due_read_receiver, 
		.write_motors = _due_write_motors
	}; 
}

static volatile uint8_t ch = 0; 


#ifdef __cplusplus
}
#endif
