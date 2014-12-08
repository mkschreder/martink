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

#include <math.h>


static void __phantom_handler(void) { while(1); }
 
void NMI_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void HardFault_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void MemManage_Handler (void) __attribute__ ((weak, alias("__phantom_handler")));
void BusFault_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void UsageFault_Handler(void) __attribute__ ((weak, alias("__phantom_handler")));
void DebugMon_Handler  (void) __attribute__ ((weak, alias("__phantom_handler")));
void SVC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PendSV_Handler    (void) __attribute__ ((weak, alias("__phantom_handler")));
void SysTick_Handler(void) { TimeTick_Increment(); }
void SUPC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RSTC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void RTT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void WDT_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void PMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void EFC1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UART_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SMC_INSTANCE_
void SMC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_SDRAMC_INSTANCE_
void SDRAMC_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PIOA_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void PIOB_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_PIOC_INSTANCE_
void PIOC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOD_INSTANCE_
void PIOD_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOE_INSTANCE_
void PIOE_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
#ifdef _SAM3XA_PIOF_INSTANCE_
void PIOF_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
//void USART0_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
//void USART1_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void USART2_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_USART3_INSTANCE_
void USART3_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void HSMCI_Handler      (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void TWI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void SPI0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_SPI1_INSTANCE_
void SPI1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void SSC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC0_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC1_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC2_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC3_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC4_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC5_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_TC2_INSTANCE_
void TC6_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC7_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void TC8_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void PWM_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void ADC_Handler        (void) __attribute__ ((weak, alias("__phantom_handler")));
void DACC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void DMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void UOTGHS_Handler     (void) __attribute__ ((weak, alias("__phantom_handler")));
void TRNG_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#ifdef _SAM3XA_EMAC_INSTANCE_
void EMAC_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
#endif
void CAN0_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
void CAN1_Handler       (void) __attribute__ ((weak, alias("__phantom_handler")));
 
#define FRONT_PIN PD3
#define RIGHT_PIN PB1
#define LEFT_PIN PB2
#define BACK_PIN PB3

#define RC_MAX 2000
#define RC_MIN 1000

#define PWM_UPDATE_DISABLE {TIMSK2 &= ~_BV(TOIE2);}
#define PWM_UPDATE_ENABLE {TIMSK2 |= _BV(TOIE2);}

#define ACCNAME mpu6050

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
}; 

static struct board _brd; 
static struct board *brd = &_brd; 
static struct uart uart;

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
/********************************************************************************
 * Controller initialisation. Call this function in main(), which is itself called
 * from Reset_Handler()
 ********************************************************************************/
void __libc_init_array(void);

void init_controller(void)
{
  /*
   * SAM System init: Initializes the PLL / clock.
   * Defined in CMSIS/ATMEL/sam3xa/source/system_sam3xa.c
   */
  SystemInit();
  /*
   * Config systick interrupt timing, core clock is in microseconds --> 1ms
   * Defined in CMSIS/CMSIS/include/core_cm3.h
   */
   SysTick_Config(SystemCoreClock / 1000);
   
  //if (SysTick_Config(SystemCoreClock / 1000)) while (1);
 
  /*
   * No watchdog now
   *
   */
  WDT_Disable(WDT);
 
  /*
   * GCC libc init, also done in Reset_Handler()
   */
}

void brd_init(void){
	/* The general init (clock, libc, watchdog disable) */
  init_controller();
  
  uart_init(38400); 
  
  uart_printf("Foo %d\n", 10); 
  
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

	/*USART_Configure(USART1, USART_MODE_ASYNCHRONOUS, 38400, SystemCoreClock); 
	USART_SetTransmitterEnabled(USART1, 1); 
	USART_SetReceiverEnabled(USART1, 1); 
	USART_EnableIt(USART1, 1); */
	
  while(1) {
    Sleep(250);
    
		uart_printf("Bar %d\n", 10); 
  
    USART_PutChar(USART1, 'D'); 
    //USART_PutChar(USART0, 'D'); 
    if(PIOB->PIO_ODSR & PIO_PB27) {
      PIOB->PIO_CODR = PIO_PB27;
    } else {
      PIOB->PIO_SODR = PIO_PB27;
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


timeout_t time_get_clock(void){
	return GetTickCount(); 
}

timeout_t time_us_to_clock(timeout_t us){
	return us / 1000; 
}


timeout_t time_clock_to_us(timeout_t clock){
	return clock * 1000; 
}


void raise(void){
	while(1); 
}

int __isnanf(float x){
	return isnan(x); 
}

float sqrtf(float x){
	return sqrt(x); 
}

#undef atan2

void __assert_fail(void){
	
}

void srand(int x) {
	
}

int __errno(void){
	return 0; 
}

#define abs(x) ((x < 0)?-x:x)
double atan2(double y, double x)
{
  float t0, t1, t2, t3, t4;

  t3 = abs(x);
  t1 = abs(y);
  t0 = max(t3, t1);
  t1 = min(t3, t1);
  t3 = (float)1.0f / t0;
  t3 = t1 * t3;

  t4 = t3 * t3;
  t0 =         - (float)0.013480470;
  t0 = t0 * t4 + (float)0.057477314;
  t0 = t0 * t4 - (float)0.121239071;
  t0 = t0 * t4 + (float)0.195635925;
  t0 = t0 * t4 - (float)0.332994597;
  t0 = t0 * t4 + (float)0.999995630;
  t3 = t0 * t3;

  t3 = (abs(y) > abs(x)) ? (float)1.570796327 - t3 : t3;
  t3 = (x < 0) ?  (float)3.141592654 - t3 : t3;
  t3 = (y < 0) ? -t3 : t3;

  return t3;
}
