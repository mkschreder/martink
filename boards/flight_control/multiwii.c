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
#include <stdlib.h>


void time_init(void); 
void uart_init(void); 
void gpio_init(void); 
void spi_init(void); 
void twi_init(void); 
void pwm_init(void); 

/*
#define FRONT_PIN PD3
#define RIGHT_PIN PB1
#define LEFT_PIN PB2
#define BACK_PIN PB3
*/

#define RC_MAX 2500
#define RC_MIN 500

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

const static uint16_t rc_defaults[6] = {1000, 1500, 1500, 1500, 1500, 1500}; 

struct multiwii_board {
	uint16_t 				rc_values[6]; 
	timestamp_t 		rc_reset_timeout; 
	pio_dev_t gpio0; 
	i2c_dev_t twi0; 
	struct bmp085 bmp;
	struct mpu6050 mpu;
	struct hmc5883l hmc; 
	struct ssd1306 ssd;
	struct fc_quad_interface interface; 
}; 

static struct multiwii_board _brd; 
static struct multiwii_board *brd = &_brd;


static void reset_rc(void){
	for(int c = 0; c < 6; c++){
		brd->rc_values[c] = rc_defaults[c]; 
	}
}

static void compute_rc_values(void){
	timestamp_t t_up, t_down;
	
	#define COMPUTE_RC_CHAN(ch) {\
		timestamp_t ticks = timestamp_ticks_to_us(t_down - t_up);\
		if(abs(brd->rc_values[ch] - ticks) > 10 && ticks > RC_MIN && ticks < RC_MAX)\
			brd->rc_values[ch] = constrain(ticks, 1000, 2000);\
	}
	uint8_t active = 0; 
	if(gpio_get_status(GPIO_RC0, &t_up, &t_down) & GP_WENT_LOW){
		active = 1; 
		COMPUTE_RC_CHAN(0); 
	}
	if(gpio_get_status(GPIO_RC1, &t_up, &t_down) & GP_WENT_LOW){
		active = 1; 
		COMPUTE_RC_CHAN(1); 
	}
	if(gpio_get_status(GPIO_RC2, &t_up, &t_down) & GP_WENT_LOW){
		active = 1; 
		COMPUTE_RC_CHAN(2); 
	}
	if(gpio_get_status(GPIO_RC3, &t_up, &t_down) & GP_WENT_LOW){
		active = 1; 
		COMPUTE_RC_CHAN(3); 
	}
	if(active){
		brd->rc_reset_timeout = timestamp_from_now_us(1000000);
	}
}

static inline void mwii_write_motors(uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	pwm0_set(front);
	pwm1_set(back);
	pwm4_set(left);
	pwm5_set(right); 
}

void mwii_process_events(void){
	compute_rc_values(); 
	
	//TIMSK0 |= _BV(TOIE0); 
	if(timestamp_expired(brd->rc_reset_timeout)){
		reset_rc(); 
	}
	
}


static void _mwii_read_accelerometer(fc_board_t self, float *x, float *y, float *z){
	double ax, ay, az; 
	//CALL(foo, 10); 
	mpu6050_getConvAcc(&brd->mpu, &ax, &ay, &az); 
	*x = ax; 
	*y = ay; 
	*z = az; 
}

static void _mwii_read_gyroscope(fc_board_t self, float *x, float *y, float *z){
	double gx, gy, gz; 
	mpu6050_getConvGyr(&brd->mpu, &gx, &gy, &gz);   
	*x = gx; 
	*y = gy; 
	*z = gz; 
	//mpu6050_getRawData(&ax, &ay, &az, x, y, z); 
}

static void _mwii_read_magnetometer(fc_board_t self, float *x, float *y, float *z){
	float mx, my, mz; 
	hmc5883l_read_adjusted(&brd->hmc, &mx, &my, &mz); 
	*x = mx; *y = my; *z = mz; 
}

static float _mwii_read_altitude(fc_board_t self){
	return bmp085_read_altitude(&brd->bmp); 
}

static long _mwii_read_pressure(fc_board_t self){
	return bmp085_read_pressure(&brd->bmp); 
}

static float _mwii_read_temperature(fc_board_t self){
	return bmp085_read_temperature(&brd->bmp); 
}

static void _mwii_write_motors(fc_board_t self,
	uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	mwii_write_motors(front, back, left, right); 
}

static uint8_t _mwii_read_receiver(fc_board_t self, 
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1) {
	*rc_thr = 		brd->rc_values[RC_THROTTLE]; 
	*rc_pitch = 	brd->rc_values[RC_PITCH]; 
	*rc_yaw = 		brd->rc_values[RC_YAW]; 
	*rc_roll = 		brd->rc_values[RC_ROLL]; 
	*rc_aux0 = 		brd->rc_values[RC_MODE]; 
	*rc_aux1 = 		brd->rc_values[RC_MODE2];
	
	//kprintf("RAW_YAW: %4d, ", brd->rc_values[RC_YAW]); 
	
	// prevent small changes when stick is not touched
	if(abs(*rc_pitch - 1500) < 20) *rc_pitch = 1500; 
	if(abs(*rc_roll - 1500) < 20) *rc_roll = 1500; 
	if(abs(*rc_yaw - 1500) < 20) *rc_yaw = 1500;

	return 0; 
}

static void _mwii_write_config(fc_board_t self, const uint8_t *data, uint16_t size){
	memory_dev_t ee = eeprom_get_memory_interface(); 
	mem_write(ee, 0, data, size); 
}

static void _mwii_read_config(fc_board_t self, uint8_t *data, uint16_t size){
	memory_dev_t ee = eeprom_get_memory_interface(); 
	mem_read(ee, 0, data, size); 
}

static serial_dev_t _mwii_get_pc_link_interface(fc_board_t self){
	return uart_get_serial_interface(0); 
}

fc_board_t mwii_get_fc_quad_interface(void){
	static struct fc_quad_interface hw = {0}; 
	static struct fc_quad_interface *ptr = 0; 
	if(!ptr){
		hw = (struct fc_quad_interface){
			.read_accelerometer = _mwii_read_accelerometer,
			.read_gyroscope = _mwii_read_gyroscope,
			.read_magnetometer = _mwii_read_magnetometer,
			.read_pressure = _mwii_read_pressure,
			.read_altitude = _mwii_read_altitude,
			.read_temperature = _mwii_read_temperature, 
			.read_receiver = _mwii_read_receiver, 
			.write_motors = _mwii_write_motors,
			
			.write_config = _mwii_write_config, 
			.read_config = _mwii_read_config, 
			
			.get_pc_link_interface = _mwii_get_pc_link_interface
		}; 
		ptr = &hw; 
	}
	// cast, but only because we never actually use any private data in multiwii
	return &ptr; 
}

void soc_init(void); 

static void mwii_calibrate(void){
	// set all outputs to maximum
	mwii_write_motors(2000, 2000, 2000, 2000); 
	// wait for the escs to initialize
	_delay_ms(3000); 
	// now step them down to mincommand within a second
	uint8_t led = 0; 
	for(int c = 2000; c > 1000; c-=10){
		mwii_write_motors(c, c, c, c); 
		_delay_ms(20); 
	}
	_delay_ms(1000); 
	// reset the motors
	mwii_write_motors(MINCOMMAND, MINCOMMAND, MINCOMMAND, MINCOMMAND); 
}

void board_init(void){
	//soc_init(); 
	time_init(); 
	uart_init(); 
	gpio_init();
	//spi_init(); 
	twi_init(); 
	pwm_init(); 
	sei(); 
	
	// first thing must enable interrupts
	kdebug("Booting MultiWii Board\n");
	
	gpio_configure(GPIO_MWII_LED, GP_OUTPUT); 
	//gpio_set(GPIO_MWII_LED); 
	
	gpio_configure(GPIO_MWII_MOTOR0, GP_OUTPUT);
	gpio_configure(GPIO_MWII_MOTOR1, GP_OUTPUT);
	gpio_configure(GPIO_MWII_MOTOR2, GP_OUTPUT);
	gpio_configure(GPIO_MWII_MOTOR3, GP_OUTPUT);
	
	gpio_configure(GPIO_MWII_RX0, GP_INPUT | GP_PULLUP | GP_PCINT);
	gpio_configure(GPIO_MWII_RX1, GP_INPUT | GP_PULLUP | GP_PCINT);
	gpio_configure(GPIO_MWII_RX2, GP_INPUT | GP_PULLUP | GP_PCINT);
	gpio_configure(GPIO_MWII_RX3, GP_INPUT | GP_PULLUP | GP_PCINT);
	
	// set initial motor speeds
	mwii_write_motors(MINCOMMAND, MINCOMMAND, MINCOMMAND, MINCOMMAND); 
	
	// calibrate escs
	//mwii_calibrate(); 
	
	brd->gpio0 = gpio_get_parallel_interface();
	
	brd->twi0 = twi_get_interface(0);
	
	//hmc5883l_init(); 
	mpu6050_init(&brd->mpu, brd->twi0, MPU6050_ADDR); 
	kdebug("MPU6050 .. %s\n", ((mpu6050_probe(&brd->mpu))?"found":"not found!")); 
	
	bmp085_init(&brd->bmp, brd->twi0, BMP085_ADDR); 
	kdebug("BMP085: reading pressure: %lu, reading temp: %d\n",
		bmp085_read_pressure(&brd->bmp), bmp085_read_temperature(&brd->bmp));
		
	reset_rc();
	
	kdebug("MultiWii initialized!\n");
	
	// let the escs init as well
	delay_us(500000L); 
}
