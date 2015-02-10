#include "openpilot_cc3d.h"
#include <stdio.h>
#include <sensors/mpu6000.h>
#include <block/serial_flash.h>
#include <fs/cfs/cfs.h>

#include <kernel.h>

#define FC_PWM_CH1 PWM_CH44
#define FC_PWM_CH2 PWM_CH43
#define FC_PWM_CH3 PWM_CH42
#define FC_PWM_CH4 PWM_CH11
#define FC_PWM_CH5 PWM_CH31
#define FC_PWM_CH6 PWM_CH23

#define FC_PWM_RC1 PWM_CH41
#define FC_PWM_RC2 PWM_CH32
#define FC_PWM_RC3 PWM_CH33
#define FC_PWM_RC4 PWM_CH34
#define FC_PWM_RC5 PWM_CH21
#define FC_PWM_RC6 PWM_CH22

#define CONFIG_FILE "config"

static struct cc3d {
	struct serial_flash flash; 
	struct mpu6000 mpu; 
	serial_dev_t spi0, spi1; 
	serial_dev_t uart0; 
} cc3d; 

	
void stm32w_flash_read(uint32_t address, void *data, uint32_t length){
	//serial_dev_t con = uart_get_serial_interface(0); 
	//printf("READ: @%x -> @%p\n", address, data); 
	serial_flash_read(&cc3d.flash, address, (uint8_t*)data, length); 
}

void stm32w_flash_write(uint32_t address, const void *data, uint32_t length){
	//serial_dev_t con = uart_get_serial_interface(0); 
	//printf("WRITE: @%x -> @%p %d\n", (int)address, data, (int)length); 
	serial_flash_write(&cc3d.flash, (int)address, (const uint8_t*)data, (int)length); 
}

void stm32w_flash_erase(uint8_t sector){
	//serial_dev_t con = uart_get_serial_interface(0); 
	//printf("ERASE: @%x\n", (int)sector); 
	serial_flash_sector_erase(&cc3d.flash, sector);
}

void cc3d_init(void){
	time_init(); 
	timestamp_init(); 
	gpio_init(); 
	
	fc_led_off(); 
	gpio_configure(GPIO_PB3, GP_OUTPUT); 
	  
	uart_init(); 
	//twi_init(); 
	spi_init(); 
	
	fc_led_on(); 
	delay_ms(500); 
	fc_led_off(); 
	delay_ms(500); 
	fc_led_on(); 
	delay_ms(500); 
	fc_led_off(); 
	
	//i2c_dev_t i2c = twi_get_interface(0); 
	cc3d.uart0 = uart_get_serial_interface(0); 
	pio_dev_t gpio = gpio_get_parallel_interface(); 
	
	//printf("Initializing sensor..\n"); 
	
	serial_dev_t spi = spi_get_serial_interface(0); 
	serial_dev_t spi2 = spi_get_serial_interface(1); 
	
	if(!spi || !spi2) {
		printf("SPI init failed!\n"); 
		while(1); 
	}
	
	mpu6000_init(&cc3d.mpu, spi, gpio, GPIO_PA4); 
	
	serial_flash_init(&cc3d.flash, spi2, GPIO_PB12); 
	
	//printf("Flash. ID: %x, Type: %x, Size: %x\n", cc3d.flash.props.id, cc3d.flash.props.type, cc3d.flash.props.size); 
	
	pwm_configure(FC_PWM_CH1, 1250, 4000); 
	pwm_configure(FC_PWM_CH2, 1250, 4000); 
	pwm_configure(FC_PWM_CH3, 1250, 4000); 
	pwm_configure(FC_PWM_CH4, 1250, 4000); 
	pwm_configure(FC_PWM_CH5, 1250, 4000); 
	pwm_configure(FC_PWM_CH6, 1250, 4000); 
	
	pwm_configure_capture(FC_PWM_RC1, 1000); 
	pwm_configure_capture(FC_PWM_RC2, 1000); 
	pwm_configure_capture(FC_PWM_RC3, 1000); 
	pwm_configure_capture(FC_PWM_RC4, 1000); 
	pwm_configure_capture(FC_PWM_RC5, 1000); 
	pwm_configure_capture(FC_PWM_RC6, 1000); 
	
	/*
	serial_dev_t con = uart_get_serial_interface(0); 
	while(1){
		uint16_t ch = serial_getc(con); 
		if(ch != SERIAL_NO_DATA) serial_putc(con, ch); 
	}*/
	/*
	{
		int fd = cfs_open("test", CFS_READ);
		char message[5] = {0}; 
		if(fd != -1) {
			cfs_read(fd, message, sizeof(message));
			cfs_close(fd);
			printf("READ: %s\n", message); 
		} else {
			printf("ERROR: could not open file for reading!\n");
		}
	}
	
	{
		int fd_write = cfs_open("test", CFS_WRITE);
		char message[5] = "alfa"; 
		if(fd_write != -1) {
			cfs_write(fd_write, message, sizeof(message));
			cfs_close(fd_write);
			printf("WRITE: %s\n", message); 
		} else {
			printf("ERROR: could not write to memory in step 2.\n");
		}
	}
	
	{
		int fd = cfs_open("test", CFS_READ);
		char message[5] = {0}; 
		if(fd != -1) {
			cfs_read(fd, message, sizeof(message));
			cfs_close(fd);
			printf("READ: %s\n", message); 
		} else {
			printf("ERROR: could not open file for reading.\n");
		}
	}
	
	cfs_remove("test");
	
	int fd_write = cfs_open("test", CFS_WRITE);
	char message[5] = "foob"; 
	if(fd_write != -1) {
		cfs_write(fd_write, message, sizeof(message));
		cfs_close(fd_write);
		printf("WRITE: %s\n", message); 
	} else {
		printf("ERROR: could not write to memory in step 2.\n");
	}

	{
		int fd = cfs_open("test", CFS_READ);
		char message[5] = {0}; 
		if(fd != -1) {
			cfs_read(fd, message, sizeof(message));
			cfs_close(fd);
			printf("READ: %s\n", message); 
		} else {
			printf("ERROR: could not write to memory in step 2.\n");
		}
	}
*/
/*
	while(1){
		gpio_set(GPIO_PB3); 
		
		printf("RC1: %d ", (int)pwm_read(FC_PWM_RC1)); 
		printf("RC2: %d ", (int)pwm_read(FC_PWM_RC2)); 
		printf("RC3: %d\n", (int)pwm_read(FC_PWM_RC3)); 
		printf("RC4: %d ", (int)pwm_read(FC_PWM_RC4)); 
		printf("RC5: %d ", (int)pwm_read(FC_PWM_RC5)); 
		printf("RC6: %d\n", (int)pwm_read(FC_PWM_RC6)); 
		
		int16_t ax, ay, az, gx, gy, gz; 
		mpu6000_getRawData(&mpu, &ax, &ay, &az, &gx, &gy, &gz); 
		
		//int data = 0; 
		///serial_flash_read(&flash, 0, (uint8_t*)&data, sizeof(data)); 
		
		//printf("DATA: %x\n", data); 
		
		if(!spi){
			printf("No SPI! \n"); 
			continue; 
		}
		
		printf("Time: %ld\n", (long int)timestamp_now()); 
		printf("Gyro: %d %d %d, Acc: %d %d %d\n", ax, ay, az, gx, gy, gz); 
		
		delay_ms(1); 
		gpio_clear(GPIO_PB3); 
		delay_ms(1); 
		//mpu6050_probe(&mpu); 
	}*/
}

static inline void cc3d_write_motors(uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	pwm_write(FC_PWM_CH1, front); 
	pwm_write(FC_PWM_CH2, back); 
	pwm_write(FC_PWM_CH3, left); 
	pwm_write(FC_PWM_CH4, right); 
}

void cc3d_process_events(void){
	
}

static int8_t _cc3d_read_sensors(fc_board_t self, struct fc_data *data){
	(void)(self); 
	
	data->flags = HAVE_ACC | HAVE_GYR; 
	memset(data, 0, sizeof(struct fc_data)); 
	
	mpu6000_getRawData(&cc3d.mpu, 
		&data->raw_acc.x, 
		&data->raw_acc.y, 
		&data->raw_acc.z,
		&data->raw_gyr.x, 
		&data->raw_gyr.y, 
		&data->raw_gyr.z
	); 
	mpu6000_convertData(&cc3d.mpu, 
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
	
	return 0; 
}

static int8_t _cc3d_write_motors(fc_board_t self,
	uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	(void)(self); 
	cc3d_write_motors(front, back, left, right); 
	return 0; 
}

static int8_t _cc3d_read_receiver(fc_board_t self, 
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1) {
	(void)(self); 
	*rc_thr = 		pwm_read(FC_PWM_RC1); 
	*rc_pitch = 	pwm_read(FC_PWM_RC2); 
	*rc_yaw = 		pwm_read(FC_PWM_RC3); 
	*rc_roll = 		pwm_read(FC_PWM_RC6); 
	*rc_aux0 = 		pwm_read(FC_PWM_RC4); 
	*rc_aux1 = 		pwm_read(FC_PWM_RC5);
	
	// prevent small changes when stick is not touched
	if(abs(*rc_pitch - 1500) < 20) *rc_pitch = 1500; 
	if(abs(*rc_roll - 1500) < 20) *rc_roll = 1500; 
	if(abs(*rc_yaw - 1500) < 20) *rc_yaw = 1500;

	return 0; 
}

static int8_t _cc3d_write_config(fc_board_t self, const uint8_t *data, uint16_t size){
	(void)(self); 
	int fd = cfs_open(CONFIG_FILE, CFS_WRITE);
	if(fd != -1) {
		cfs_write(fd, data, size);
		cfs_close(fd);
	} else {
		return -1; 
	}
	return 0; 
}

static int8_t _cc3d_read_config(fc_board_t self, uint8_t *data, uint16_t size){
	(void)(self); 
	int fd = cfs_open(CONFIG_FILE, CFS_READ);
	if(fd != -1) {
		cfs_read(fd, data, size);
		cfs_close(fd);
	} else {
		return -1; 
	}
	return 0; 
}

static serial_dev_t _cc3d_get_pc_link_interface(fc_board_t self){
	(void)(self); 
	return cc3d.uart0; 
}

fc_board_t cc3d_get_fc_quad_interface(void){
	static struct fc_quad_interface hw = {0}; 
	static struct fc_quad_interface *ptr = 0; 
	if(!ptr){
		hw = (struct fc_quad_interface){
			.read_sensors = _cc3d_read_sensors, 
			.read_receiver = _cc3d_read_receiver, 
			
			.write_motors = _cc3d_write_motors,
			
			.write_config = _cc3d_write_config, 
			.read_config = _cc3d_read_config, 
			
			.get_pc_link_interface = _cc3d_get_pc_link_interface
		}; 
		ptr = &hw; 
	}
	return &ptr;  
}
