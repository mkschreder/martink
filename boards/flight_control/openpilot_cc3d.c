#include "openpilot_cc3d.h"
#include <stdio.h>

#include <kernel.h>
#include <sensors/mpu6000.h>
#include <block/serial_flash.h>
#include <fs/cfs/cfs.h>


#define CONFIG_FILE "config"

#define CC3D_MAINPORT_UART_ID 0
#define CC3D_FLEXIPORT_UART_ID 2
#define CC3D_FLEXIPORT_I2C_ID 1
#define CC3D_DEFAULT_UART_BAUDRATE 38400

static struct cc3d {
	struct serial_flash flash; 
	struct mpu6000 mpu; 
	serial_dev_t spi0, spi1; 
	serial_dev_t uart0; 
} cc3d; 

	
void stm32w_flash_read(uint32_t address, void *data, uint32_t length){
	//serial_dev_t con = uart_get_serial_interface(0); 
	//printf("READ: @%x -> @%p\n", (int)address, data); 
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
	
	cc3d_led_off(); 
	gpio_configure(GPIO_PB3, GP_OUTPUT); 
	
	cc3d_led_on(); 
	uart_init(CC3D_MAINPORT_UART_ID, CC3D_DEFAULT_UART_BAUDRATE); 
	uart_init(CC3D_FLEXIPORT_UART_ID, 57600); 
	/*serial_dev_t sp = uart_get_serial_interface(CC3D_FLEXIPORT_UART_ID); 
	serial_printf(sp, "AT"); 
	delay_ms(500); 
	serial_printf(sp, "AT+NAMEcc3dd"); 
	delay_ms(500); 
	serial_printf(sp, "AT+BAUD6"); 
	delay_ms(500); 
	uart_set_baudrate(CC3D_FLEXIPORT_UART_ID, 38400); */
	
	cc3d_led_off(); 
	/*while(1){
		serial_printf(uart_get_serial_interface(0), "Running!\n"); 
		serial_printf(sp, "Hello World!\n"); 
	}*/
	
	//twi_init(); 
	spi_init(); 
	
	cc3d_led_on(); 
	delay_ms(500); 
	cc3d_led_off(); 
	delay_ms(500); 
	cc3d_led_on(); 
	delay_ms(500); 
	cc3d_led_off(); 
	
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
	
	pwm_configure(CC3D_OUT_PWM1, 950, 4000); 
	pwm_configure(CC3D_OUT_PWM2, 950, 4000); 
	pwm_configure(CC3D_OUT_PWM3, 950, 4000); 
	pwm_configure(CC3D_OUT_PWM4, 950, 4000); 
	pwm_configure(CC3D_OUT_PWM5, 950, 4000); 
	pwm_configure(CC3D_OUT_PWM6, 950, 4000); 
	
	pwm_configure_capture(CC3D_IN_PWM1, 1000); 
	pwm_configure_capture(CC3D_IN_PWM2, 1000); 
	pwm_configure_capture(CC3D_IN_PWM3, 1000); 
	pwm_configure_capture(CC3D_IN_PWM4, 1000); 
	pwm_configure_capture(CC3D_IN_PWM5, 1000); 
	pwm_configure_capture(CC3D_IN_PWM6, 1000); 
	
}


uint16_t cc3d_read_pwm(cc3d_input_pwm_id_t chan){
	return pwm_read(chan); 
}
/*
int8_t cc3d_read_receiver(uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1){
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
}*/

void cc3d_write_pwm(cc3d_output_pwm_id_t ch, uint16_t data){
	pwm_write(ch, data); 
	/*
	pwm_write(FC_PWM_CH1, front); 
	pwm_write(FC_PWM_CH2, back); 
	pwm_write(FC_PWM_CH3, left); 
	pwm_write(FC_PWM_CH4, right); */
}


void cc3d_configure_flexiport(cc3d_flexi_port_mode_t mode){
	switch(mode){
		case CC3D_FLEXIPORT_UART: 
			twi_deinit(CC3D_FLEXIPORT_I2C_ID); 
			uart_init(CC3D_FLEXIPORT_UART_ID, CC3D_DEFAULT_UART_BAUDRATE); 
			break; 
		case CC3D_FLEXIPORT_I2C: 
			uart_deinit(CC3D_FLEXIPORT_UART_ID); 
			twi_init(CC3D_FLEXIPORT_UART_ID); 
			break; 
	};
}

serial_dev_t cc3d_get_mainport_serial_interface(void){
	return uart_get_serial_interface(CC3D_MAINPORT_UART_ID); 
}

serial_dev_t cc3d_get_flexiport_serial_interface(void){
	return uart_get_serial_interface(CC3D_FLEXIPORT_UART_ID); 
}

i2c_dev_t cc3d_get_flexiport_i2c_interface(void){
	return twi_get_interface(CC3D_FLEXIPORT_I2C_ID); 
}

/// reads last measured acceleration in G
void cc3d_read_acceleration_g(float *ax, float *ay, float *az){
	int16_t x, y, z; 
	mpu6000_readRawAcc(&cc3d.mpu, &x, &y, &z); 
	mpu6000_convertAcc(&cc3d.mpu, x, y, z, ax, ay, az); 
}

/// reads last measured angular velocity in degrees / sec
void cc3d_read_angular_velocity_dps(float *gyrx, float *gyry, float *gyrz){
	int16_t x, y, z; 
	mpu6000_readRawGyr(&cc3d.mpu, &x, &y, &z); 
	mpu6000_convertGyr(&cc3d.mpu, x, y, z, gyrx, gyry, gyrz); 
} 

static int8_t cc3d_read_sensors(struct fc_data *data){
	data->flags = HAVE_ACC | HAVE_GYR; 
	memset(data, 0, sizeof(struct fc_data)); 
	
	mpu6000_readRawAcc(&cc3d.mpu, 
		&data->raw_acc.x, 
		&data->raw_acc.y, 
		&data->raw_acc.z
	); 
	mpu6000_convertAcc(&cc3d.mpu, 
		data->raw_acc.x, 
		data->raw_acc.y, 
		data->raw_acc.z, 
		&data->acc_g.x, 
		&data->acc_g.y, 
		&data->acc_g.z
	); 
	mpu6000_readRawGyr(&cc3d.mpu, 
		&data->raw_gyr.x, 
		&data->raw_gyr.y, 
		&data->raw_gyr.z
	); 
	mpu6000_convertAcc(&cc3d.mpu, 
		data->raw_gyr.x, 
		data->raw_gyr.y, 
		data->raw_gyr.z, 
		&data->gyr_deg.x,
		&data->gyr_deg.y,
		&data->gyr_deg.z
	); 
	return 0; 
}

int8_t cc3d_write_config(const uint8_t *data, uint16_t size){
	uint8_t sector[512]; 
	int32_t addr = -1; 
	uint16_t sect = 0; 
	serial_flash_read(&cc3d.flash, 0, sector, 512); 
	// find first free sector
	for(int c = 0; c < 512; c++){
		for(int j = 0; j < 8; j++){
			if(!(sector[c] & (1 << j))){
				sect = (c * 8 + j); 
				addr = sect * 512; 
				break; 
			}
		}
		if(addr != -1) break; 
	}
	
	if(addr == -1) return -1; 
	
	printf("Found free sector %x at %x\n", (int)sect, (int)addr); 
	
	unsigned c = 0; 
	do {
		uint16_t len = ((size - c) > 512)?512:(size - c); 
		serial_flash_write(&cc3d.flash, sect * 512, data + c, len); 
		uint8_t flags = 0; 
		serial_flash_read(&cc3d.flash, sect / 8, &flags, 1); 
		flags |= (1 << (sect % 8)); 
		serial_flash_write(&cc3d.flash, (sect / 8), &flags, 1); 
		printf("Wrote block to sector %x\n", sect); 
		sect++; 
		c += 512; 
	} while(c < size); 
	
	//serial_flash_sector_erase(&cc3d.flash, 0); 
	//serial_flash_write(&cc3d.flash, 0, data, size); 
	return 0; 
	/*
	int fd = cfs_open(CONFIG_FILE, CFS_WRITE);
	if(fd != -1) {
		cfs_write(fd, data, size);
		cfs_close(fd);
	} else {
		return -1; 
	}
	return 0; */
}

int8_t cc3d_read_config(uint8_t *data, uint16_t size){
	uint8_t sector[512]; 
	int32_t addr = -1; 
	int16_t sect = 0; 
	serial_flash_read(&cc3d.flash, 0, sector, 512); 
	// find first free sector
	for(int c = 0; c < 512; c++){
		for(int j = 0; j < 8; j++){
			if(!(sector[c] & (1 << j))){
				sect = (c * 8 + j) - 1; 
				addr = sect * 512; 
				break; 
			}
		}
		if(addr != -1) break;
	}
	if(addr < 0) return -1; 
	
	printf("Found data at sector %x\n", sect); 
	serial_flash_read(&cc3d.flash, addr, data, size); 
	
	return 0; 
	/*
	int fd = cfs_open(CONFIG_FILE, CFS_READ);
	if(fd != -1) {
		cfs_read(fd, data, size);
		cfs_close(fd);
	} else {
		return -1; 
	}
	return 0; */
}

void cc3d_process_events(void){
	
}

static int8_t _cc3d_read_sensors(fc_board_t self, struct fc_data *data){
	(void)(self); 
	return cc3d_read_sensors(data); 
}

static int8_t _cc3d_write_motors(fc_board_t self,
	uint16_t front, uint16_t back, uint16_t left, uint16_t right){
	(void)(self); 
	cc3d_write_pwm(CC3D_OUT_PWM1, front); 
	cc3d_write_pwm(CC3D_OUT_PWM2, back); 
	cc3d_write_pwm(CC3D_OUT_PWM3, left); 
	cc3d_write_pwm(CC3D_OUT_PWM4, right); 
	return 0; 
}

static int8_t _cc3d_read_receiver(fc_board_t self, 
		uint16_t *rc_thr, uint16_t *rc_yaw, uint16_t *rc_pitch, uint16_t *rc_roll,
		uint16_t *rc_aux0, uint16_t *rc_aux1) {
	(void)(self); 
	*rc_thr = 	cc3d_read_pwm(CC3D_IN_PWM1); 
	*rc_yaw = 	cc3d_read_pwm(CC3D_IN_PWM2); 
	*rc_pitch = cc3d_read_pwm(CC3D_IN_PWM3); 
	*rc_roll = 	cc3d_read_pwm(CC3D_IN_PWM4); 
	*rc_aux0 = 	cc3d_read_pwm(CC3D_IN_PWM5); 
	*rc_aux1 = 	cc3d_read_pwm(CC3D_IN_PWM6); 
	
	// prevent small changes when stick is not touched
	if(abs(*rc_pitch - 1500) < 20) *rc_pitch = 1500; 
	if(abs(*rc_roll - 1500) < 20) *rc_roll = 1500; 
	if(abs(*rc_yaw - 1500) < 20) *rc_yaw = 1500;
	return 0; 
}

static int8_t _cc3d_write_config(fc_board_t self, const uint8_t *data, uint16_t size){
	(void)(self); 
	return cc3d_write_config(data, size); 
}

static int8_t _cc3d_read_config(fc_board_t self, uint8_t *data, uint16_t size){
	(void)(self); 
	return cc3d_read_config(data, size); 
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
