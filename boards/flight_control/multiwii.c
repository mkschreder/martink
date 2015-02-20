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
#include <sensors/hcsr04.h>
#include <disp/ssd1306.h>
#include <util/serial_debugger.h>

#include <radio/nrf24l01.h>

#include <math.h>
#include <string.h>
#include <stdlib.h>

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

#define GPIO_RC0 					GPIO_PD2
#define GPIO_RC1 					GPIO_PD4
#define GPIO_RC2 					GPIO_PD7
#define GPIO_RC3 					GPIO_PB0

#define GPIO_BATTERY_MON	GPIO_PC0 //A0
#define GPIO_MWII_HCSR_TRIGGER GPIO_PB1
#define GPIO_MWII_HCSR_ECHO 	GPIO_PB2

enum {
	RC_THROTTLE = 0, 
	RC_YAW 			= 1, 
	RC_PITCH 		= 2,
	RC_ROLL 		= 3,
	RC_MODE 		= 4,
	RC_MODE2		= 5
};

static const uint16_t rc_defaults[6] = {1000, 1500, 1500, 1500, 1500, 1500}; 

struct rc_input {
	struct pin_state ps; 
	struct pt thread; 
	timestamp_t last_activity; 
};

struct multiwii_board {
	uint16_t 				rc_values[6]; 
	pio_dev_t 			gpio0; 
	i2c_dev_t 			twi0; 
	struct bmp085 	bmp;
	struct mpu6050 	mpu;
	struct hmc5883l hmc; 
	struct ssd1306 	ssd;
	struct hcsr04 	hcsr; 
	int16_t acc_bias_x, acc_bias_y, acc_bias_z; 
	struct pt 			uthread; 
	timestamp_t 		time; // time keeping
	struct rc_input rc_inputs[4]; 
	struct fc_quad_interface interface; 
}; 

static struct multiwii_board _brd; 
static struct multiwii_board *brd = &_brd;

#define COMPUTE_RC_CHAN(ch) {\
	timestamp_t ticks = timestamp_ticks_to_us(brd->rc_inputs[ch].ps.t_down - brd->rc_inputs[ch].ps.t_up);\
	if(abs(brd->rc_values[ch] - ticks) > 10 && ticks > RC_MIN && ticks < RC_MAX){\
		brd->rc_values[ch] = constrain(ticks, 1000, 2000);\
		brd->rc_inputs[ch].last_activity = timestamp_now(); \
	}\
}

static PT_THREAD(rc_thread(uint8_t c)){
	static const gpio_pin_t pins[] = {GPIO_RC0, GPIO_RC1, GPIO_RC2, GPIO_RC3}; 
	PT_BEGIN(&brd->rc_inputs[c].thread); 
	while(1){
		gpio_start_read(pins[c], &brd->rc_inputs[c].ps, GP_READ_PULSE_P); 
		PT_WAIT_WHILE(&brd->rc_inputs[c].thread, gpio_pin_busy(pins[c])); 
		COMPUTE_RC_CHAN(c); 
	}
	PT_END(&brd->rc_inputs[c].thread); 
}

void mwii_write_motors(uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	mwii_write_pwm(MWII_OUT_PWM0, front);
	mwii_write_pwm(MWII_OUT_PWM1, back);
	mwii_write_pwm(MWII_OUT_PWM2, left);
	mwii_write_pwm(MWII_OUT_PWM3, right); 
}

/*
static void mwii_calibrate_mpu6050(void){
	// calibrate gyro offset
	int16_t ax, ay, az, gx, gy, gz; 
	
	mpu6050_setTCXGyroOffset(&_brd.mpu, 0); //14
	mpu6050_setTCYGyroOffset(&_brd.mpu, 0); //20
	mpu6050_setTCZGyroOffset(&_brd.mpu, 0); //-49
	mpu6050_setXGyroOffset(&_brd.mpu, 0); //14
	mpu6050_setYGyroOffset(&_brd.mpu, 0); //20
	mpu6050_setZGyroOffset(&_brd.mpu, 0); //-49
	
	int32_t aax = 0, aay= 0, aaz = 0, ggx = 0, ggy = 0, ggz = 0; 
	static const int iterations = 100; 
	for(int c = 0; c < iterations; c++){
		mpu6050_readRawAcc(&_brd.mpu, &ax, &ay, &az); 
		mpu6050_readRawGyr(&_brd.mpu, &gx, &gy, &gz); 
		aax += ax; aay += ay; aaz += az; 
		ggx += gx; ggy += gy; ggz += gz; 
		delay_us(10); 
	}
	brd->acc_bias_x = aax / iterations; 
	brd->acc_bias_y = aay / iterations; 
	brd->acc_bias_z = (aaz / iterations) + 16384; 
	
	mpu6050_setXGyroOffset(&_brd.mpu, -(int16_t)(ggx / iterations * 2) | 1); //14
	mpu6050_setYGyroOffset(&_brd.mpu, -(int16_t)(ggy / iterations * 2) | 1); //20
	mpu6050_setZGyroOffset(&_brd.mpu, -(int16_t)(ggz / iterations * 2) | 1); //-49
	
}*/

static void _serial_fd_putc(int c, FILE *stream){
	serial_dev_t dev = fdev_get_udata(stream); 
	serial_putc(dev, c); 
}

static int _serial_fd_getc(FILE *stream){
	serial_dev_t dev = fdev_get_udata(stream); 
	return serial_getc(dev) & 0xff; 
}

static FILE uart_fd = FDEV_SETUP_STREAM(_serial_fd_putc, _serial_fd_getc, _FDEV_SETUP_RW);

void mwii_init(void){
	//soc_init(); 
	time_init(); 
	uart_init(0, 38400); 
	gpio_init();
	//spi_init(); 
	twi_init(0); 
	pwm_init(); 
	//adc0_init_default(); 
	sei(); 
	
	PT_INIT(&brd->rc_inputs[0].thread); 
	PT_INIT(&brd->rc_inputs[1].thread); 
	PT_INIT(&brd->rc_inputs[2].thread); 
	PT_INIT(&brd->rc_inputs[3].thread); 
	PT_INIT(&brd->uthread); 
	
	// setup printf stuff (specific to avr-libc)
	fdev_set_udata(&uart_fd, uart_get_serial_interface(0)); 
	stdout = &uart_fd; 
	stderr = &uart_fd; 
	
	/*
	// setup stdout and stderr (avr-libc specific) 
	fdev_setup_stream(stdout, _serial_fd_putc, _serial_fd_getc, _FDEV_SETUP_RW); 
	fdev_set_udata(stdout, uart_get_serial_interface(0)); 
	fdev_setup_stream(stderr, _serial_fd_putc, _serial_fd_getc, _FDEV_SETUP_RW); 
	fdev_set_udata(stderr, uart_get_serial_interface(0)); 
*/
	// first thing must enable interrupts
	//printf("BOOT\n");
	
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
	
	// calibrate escs
	//mwii_calibrate_escs(); 
	
	// set initial motor speeds
	mwii_write_motors(MINCOMMAND, MINCOMMAND, MINCOMMAND, MINCOMMAND); 
	
	brd->gpio0 = gpio_get_parallel_interface();
	
	brd->twi0 = twi_get_interface(0);
	
	/* 
	// I2C scanner 
	for(int c = 0; c < 255; c++){
		uint8_t buf[2]; 
		i2c_start_read(brd->twi0, c, buf, 1); 
		if(i2c_stop(brd->twi0) == 1 && c & 1){
			kprintf("Device %x@i2c\n", c >> 1); 
		}
		delay_us(10000); 
	}
	*/
	gpio_set(GPIO_MWII_LED);
	 
	mpu6050_init(&brd->mpu, brd->twi0, MPU6050_ADDR); 
	kdebug("MPU6050: %s\n", ((mpu6050_probe(&brd->mpu))?"found":"not found!")); 
	
	bmp085_init(&brd->bmp, brd->twi0, BMP085_ADDR); 
	kdebug("BMP085: found\n");
	
	hmc5883l_init(&brd->hmc, brd->twi0, HMC5883L_ADDR);
	
	gpio_clear(GPIO_MWII_LED); 
	
	hcsr04_init(&brd->hcsr, brd->gpio0, GPIO_MWII_HCSR_TRIGGER, GPIO_MWII_HCSR_ECHO); 
	
	//mwii_calibrate_mpu6050(); 
	// let the escs init as well
	delay_us(500000L); 
}

/// writes a pwm pulse to specified pwm channel
void mwii_write_pwm(mwii_out_pwm_channel_t chan, uint16_t value){
	switch(chan){
		case MWII_OUT_PWM0: pwm0_set(value); break; 
		case MWII_OUT_PWM1: pwm1_set(value);  break; 
		case MWII_OUT_PWM2: pwm4_set(value);  break; 
		case MWII_OUT_PWM3: pwm5_set(value);  break; 
	}
}

/// reads most recent captured pwm pulse length from channel
uint16_t mwii_read_pwm(mwii_in_pwm_channel_t chan){
	switch(chan){
		case MWII_IN_PWM0: return brd->rc_values[0]; 
		case MWII_IN_PWM1: return brd->rc_values[1]; 
		case MWII_IN_PWM2: return brd->rc_values[2]; 
		case MWII_IN_PWM3: return brd->rc_values[3]; 
	}
	return 0; 
}

/// gets main i2c interface of the board (it only has one) 
i2c_dev_t mwii_get_i2c_interface(void){
	return twi_get_interface(0); 
}

/// gets main usart interface
serial_dev_t mwii_get_uart_interface(void){
	return uart_get_serial_interface(0); 
}

//**********************
// READING SENSOR DATA
//**********************
/// reads last measured acceleration in G
void mwii_read_acceleration_g(float *ax, float *ay, float *az){
	int16_t x, y, z; 
	mpu6050_readRawAcc(&_brd.mpu, &x, &y, &z); 
	mpu6050_convertAcc(&_brd.mpu, x, y, z, ax, ay, az); 
}

/// reads last measured angular velocity in degrees / sec
void mwii_read_angular_velocity_dps(float *gyrx, float *gyry, float *gyrz){
	int16_t x, y, z; 
	mpu6050_readRawGyr(&_brd.mpu, &x, &y, &z); 
	mpu6050_convertGyr(&_brd.mpu, x, y, z, gyrx, gyry, gyrz); 
}

/// reads last measured magnetic field in ? 
void mwii_read_magnetic_field(float *mx, float *my, float *mz){
	int16_t x, y, z; 
	hmc5883l_readRawMag(&_brd.hmc, &x, &y, &z); 
	hmc5883l_convertMag(&_brd.hmc, x, y, z, mx, my, mz); 
}

/// reads temperature in degrees C
float mwii_read_temperature_c(void){
	return bmp085_read_temperature(&brd->bmp); 
}

/// reads pressure in pascal
float mwii_read_pressure_pa(void){
	return bmp085_read_pressure(&brd->bmp); 
}


void mwii_write_config(const uint8_t *data, uint8_t size){
	memory_dev_t ee = eeprom_get_memory_interface(); 
	mem_write(ee, 0, data, size); 
}

void mwii_read_config(uint8_t *data, uint8_t size){
	memory_dev_t ee = eeprom_get_memory_interface(); 
	mem_read(ee, 0, data, size); 
}

//************************
// UTILITIES
//************************

/// schedules an esc calibration to be done upon next powerup
void mwii_calibrate_escs_on_reboot(void){
	// TODO: save a flag and check it upon reboot
}

static PT_THREAD(_mwii_update_thread(void)){
	struct pt *thr = &brd->uthread; 
	PT_BEGIN(thr); 
	
	while(1){
		PT_WAIT_UNTIL(thr, timestamp_expired(brd->time)); 
		
		// reset rc inputs
		for(int c = 0; c < 4; c++){
			rc_thread(c); 
			if(timestamp_expired(brd->rc_inputs[c].last_activity + timestamp_us_to_ticks(500000))){
				brd->rc_values[c] = rc_defaults[c]; 
			}
		}
		brd->time = timestamp_from_now_us(10000); 
	}
	
	PT_END(thr); 
}

void mwii_process_events(void){
	_mwii_update_thread(); 
	
	bmp085_update(&_brd.bmp); 
	hmc5883l_update(&_brd.hmc); 
	mpu6050_update(&_brd.mpu); 
}

void mwii_read_sensors(struct fc_data *data){
	data->flags = 0xffff; // all
	mpu6050_readRawAcc(&_brd.mpu, 
		&data->raw_acc.x, 
		&data->raw_acc.y, 
		&data->raw_acc.z); 
	mpu6050_convertAcc(&_brd.mpu, 
		data->raw_acc.x, 
		data->raw_acc.y, 
		data->raw_acc.z, 
		&data->acc_g.x, 
		&data->acc_g.y, 
		&data->acc_g.z
	); 
	mpu6050_readRawGyr(&_brd.mpu, 
		&data->raw_gyr.x, 
		&data->raw_gyr.y, 
		&data->raw_gyr.z
	); 
	mpu6050_convertGyr(&_brd.mpu, 
		data->raw_gyr.x, 
		data->raw_gyr.y, 
		data->raw_gyr.z, 
		&data->gyr_deg.x,
		&data->gyr_deg.y,
		&data->gyr_deg.z
	); 
	/*
	hmc5883l_readRawMag(&_brd.hmc, 
		&data->raw_mag.x, 
		&data->raw_mag.y, 
		&data->raw_mag.z
	); 
	hmc5883l_convertMag(&_brd.hmc, 
		data->raw_mag.x, 
		data->raw_mag.y, 
		data->raw_mag.z, 
		&data->mag.x, 
		&data->mag.y, 
		&data->mag.z
	); 
	*/
	int16_t sonar = hcsr04_read_distance_in_cm(&brd->hcsr); 
	data->atmospheric_altitude = bmp085_read_altitude(&brd->bmp); 
	data->sonar_altitude = (sonar > 0)?((float)sonar / 100.0):-1; 
	data->temperature = bmp085_read_temperature(&brd->bmp); 
	data->pressure = bmp085_read_pressure(&brd->bmp); 
	
	//data->vbat = (adc0_read_cached(2) / 65535.0);
}

void mwii_read_receiver(
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
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
}

//***********************
// INTERFACE 
//***********************

/*
static int8_t _mwii_write_motors(fc_board_t self,
	uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	(void)(self); 
	mwii_write_motors(front, back, left, right); 
	return 0; 
}

static int8_t _mwii_read_receiver(fc_board_t self, 
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1) {
	(void)(self); 
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

static int8_t _mwii_write_config(fc_board_t self, const uint8_t *data, uint16_t size){
	(void)(self); 
	memory_dev_t ee = eeprom_get_memory_interface(); 
	mem_write(ee, 0, data, size); 
	return 0; 
}

static int8_t _mwii_read_config(fc_board_t self, uint8_t *data, uint16_t size){
	(void)(self); 
	memory_dev_t ee = eeprom_get_memory_interface(); 
	mem_read(ee, 0, data, size); 
	return 0; 
}

static serial_dev_t _mwii_get_pc_link_interface(fc_board_t self){
	(void)(self); 
	return uart_get_serial_interface(0); 
}

fc_board_t mwii_get_fc_quad_interface(void){
	static struct fc_quad_interface hw = {0}; 
	static struct fc_quad_interface *ptr = 0; 
	if(!ptr){
		hw = (struct fc_quad_interface){
			.read_sensors = _mwii_read_sensors, 
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
*/
/*
static void _mwii_calibrate_escs(void){
	// set all outputs to maximum
	mwii_write_motors(2000, 2000, 2000, 2000); 
	// wait for the escs to initialize
	_delay_ms(3000); 
	// now step them down to mincommand within a second
	for(int c = 2000; c > 1000; c-=10){
		mwii_write_motors(c, c, c, c); 
		_delay_ms(20); 
	}
	_delay_ms(500); 
	// reset the motors
	mwii_write_motors(MINCOMMAND, MINCOMMAND, MINCOMMAND, MINCOMMAND); 
}
*/
