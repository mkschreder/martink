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

static const uint16_t rc_defaults[6] = {1000, 1500, 1500, 1500, 1500, 1500}; 

struct multiwii_board {
	uint16_t 				rc_values[6]; 
	timestamp_t 		rc_reset_timeout; 
	pio_dev_t 			gpio0; 
	i2c_dev_t 			twi0; 
	struct bmp085 	bmp;
	struct mpu6050 	mpu;
	struct hmc5883l hmc; 
	struct ssd1306 	ssd;
	struct hcsr04 	hcsr; 
	int16_t acc_bias_x, acc_bias_y, acc_bias_z; 
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

static int8_t _mwii_read_sensors(fc_board_t self, struct fc_data *data){
	(void)(self); 
	data->flags = 0xffff; // all
	mpu6050_getRawData(&_brd.mpu, 
		&data->raw_acc.x, 
		&data->raw_acc.y, 
		&data->raw_acc.z,
		&data->raw_gyr.x, 
		&data->raw_gyr.y, 
		&data->raw_gyr.z
	); 
	/*data->raw_acc.x -= brd->acc_bias_x; 
	data->raw_acc.y -= brd->acc_bias_y; 
	data->raw_acc.z -= brd->acc_bias_z; 
	*/
	mpu6050_convertData(&_brd.mpu, 
		data->raw_acc.x, 
		data->raw_acc.y, 
		data->raw_acc.z, 
		data->raw_gyr.x, 
		data->raw_gyr.y, 
		data->raw_gyr.z, 
		&data->acc_g.x, 
		&data->acc_g.y, 
		&data->acc_g.z,
		&data->gyr_deg.x,
		&data->gyr_deg.y,
		&data->gyr_deg.z
	); 
	hmc5883l_read_raw(&_brd.hmc, 
		&data->raw_mag.x, 
		&data->raw_mag.y, 
		&data->raw_mag.z
	); 
	hmc5883l_convertData(&_brd.hmc, 
		data->raw_mag.x, 
		data->raw_mag.y, 
		data->raw_mag.z, 
		&data->mag.x, 
		&data->mag.y, 
		&data->mag.z
	); 
	
	int16_t sonar = hcsr04_read_distance_in_cm(&brd->hcsr); 
	data->atmospheric_altitude = bmp085_read_altitude(&brd->bmp); 
	data->sonar_altitude = (sonar > 0)?((float)sonar / 100.0):-1; 
	data->temperature = bmp085_read_temperature(&brd->bmp); 
	data->pressure = bmp085_read_pressure(&brd->bmp); 
	data->vbat = (adc0_read_immediate(2) / 65535.0);
	return 0; 
}

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


void mwii_calibrate_escs(void){
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

static void mwii_calibrate_mpu6050(void){
	// calibrate gyro offset
	int16_t ax, ay, az, gx, gy, gz; 
	
	mpu6050_setTCXGyroOffset(&_brd.mpu, 0); //14
	mpu6050_setTCYGyroOffset(&_brd.mpu, 0); //20
	mpu6050_setTCZGyroOffset(&_brd.mpu, 0); //-49
	mpu6050_setXGyroOffset(&_brd.mpu, 0); //14
	mpu6050_setYGyroOffset(&_brd.mpu, 0); //20
	mpu6050_setZGyroOffset(&_brd.mpu, 0); //-49
	
	/*
	int16_t bias_x = mpu6050_getXAccOffset(&_brd.mpu); 
	int16_t bias_y = mpu6050_getYAccOffset(&_brd.mpu); 
	int16_t bias_z = mpu6050_getZAccOffset(&_brd.mpu); 
	*/
	/*mpu6050_setXAccOffset(&_brd.mpu, 0); 
	mpu6050_setYAccOffset(&_brd.mpu, 0); 
	mpu6050_setZAccOffset(&_brd.mpu, 0); 
	*/
	int32_t aax = 0, aay= 0, aaz = 0, ggx = 0, ggy = 0, ggz = 0; 
	static const int iterations = 100; 
	for(int c = 0; c < iterations; c++){
		mpu6050_getRawData(&_brd.mpu, &ax, &ay, &az, &gx, &gy, &gz); 
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
	
	// Construct the accelerometer biases for push to the hardware accelerometer bias registers. These registers contain
	// factory trim values which must be added to the calculated accelerometer biases; on boot up these registers will hold
	// non-zero values. In addition, bit 0 of the lower byte must be preserved since it is used for temperature
	// compensation calculations. Accelerometer bias registers expect bias input as 2048 LSB per g, so that
	// the accelerometer biases calculated above must be divided by 8.
	
	// Subtract calculated averaged accelerometer bias scaled to 2048 LSB/g (16 g full scale)
	// preserve temperature compensation bit when writing back to accelerometer bias registers
	//bias_x = (int16_t)(bias_x - (aax / iterations / 16384.0 * 2048)) | 1; 
	//bias_y = (int16_t)(bias_y - (aay / iterations / 16384.0 * 2048)) | 1; 
	//bias_z = (int16_t)(bias_z - (aaz / iterations / 16384.0 * 2048)) | 1; 
	
	//mpu6050_setXAccOffset(&_brd.mpu, -(int16_t)(bias_x)); 
	//mpu6050_setYAccOffset(&_brd.mpu, -(int16_t)(bias_y)); 
	//mpu6050_setZAccOffset(&_brd.mpu, -(int16_t)(aaz)); 
	
	/*mpu6050_setXAccOffset(&_brd.mpu, 0x0100); 
	mpu6050_setYAccOffset(&_brd.mpu, 0x0100); 
	mpu6050_setZAccOffset(&_brd.mpu, 0x0100); */
	
	//mpu6050_setXGyroOffset(&_brd.mpu, -15 * 2); //14
	//mpu6050_setYGyroOffset(&_brd.mpu, -20 * 2); //20
	//mpu6050_setZGyroOffset(&_brd.mpu, 54 * 2); //-49
	//mpu6050_setZAccOffset(&_brd.mpu, -20); // 15818
	/*
	mpu6050_getRawData(&_brd.mpu, &ax, &ay, &az, &gx, &gy, &gz); 
	
	mpu6050_setXGyroOffset(&_brd.mpu, gx); 
	mpu6050_setYGyroOffset(&_brd.mpu, gy); 
	mpu6050_setZGyroOffset(&_brd.mpu, gz); */
}

void mwii_init(void){
	//soc_init(); 
	time_init(); 
	uart_init(); 
	gpio_init();
	//spi_init(); 
	twi_init(); 
	pwm_init(); 
	adc0_init_default(); 
	sei(); 
	
	// first thing must enable interrupts
	kdebug("BOOT\n");
	
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
	mwii_calibrate_escs(); 
	
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
	//uint32_t hmcid = hmc5883l_read_id(&brd->hmc);  
	//kdebug("HMC5883: %c%c%c\n", (uint8_t)(hmcid >> 16), (uint8_t)(hmcid >> 8), (uint8_t)hmcid); 
	
	gpio_clear(GPIO_MWII_LED); 
	
	hcsr04_init(&brd->hcsr, brd->gpio0, GPIO_MWII_HCSR_TRIGGER, GPIO_MWII_HCSR_ECHO); 
	
	reset_rc();
	
	mwii_calibrate_mpu6050(); 
	// let the escs init as well
	delay_us(500000L); 
}
