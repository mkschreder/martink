#include "openpilot_cc3d.h"
#include <sensors/mpu6000.h>
#include <block/serial_flash.h>

#include <kernel.h>

#define FC_PWM_CH1 PWM_CH44
#define FC_PWM_CH2 PWM_CH43
#define FC_PWM_CH3 PWM_CH42
#define FC_PWM_CH4 PWM_CH11
#define FC_PWM_CH5 PWM_CH31
#define FC_PWM_CH6 PWM_CH23

void cc3d_init(void){
	time_init(); 
	timestamp_init(); 
	gpio_init(); 
	uart_init(); 
	//twi_init(); 
	spi_init(); 
	
	gpio_configure(GPIO_PB3, GP_OUTPUT); 
	
	gpio_set(GPIO_PB3); 
	delay_ms(1000); 
	gpio_clear(GPIO_PB3); 
	delay_ms(1000); 
	
	struct mpu6000 mpu; 
	struct serial_flash flash; 
	
	//i2c_dev_t i2c = twi_get_interface(0); 
	serial_dev_t con = uart_get_serial_interface(0); 
	pio_dev_t gpio = gpio_get_parallel_interface(); 
	
	serial_printf(con, "Initializing sensor..\n"); 
	
	serial_dev_t spi = spi_get_serial_interface(0); 
	serial_dev_t spi2 = spi_get_serial_interface(1); 
	
	if(!spi || !spi2) {
		serial_printf(con, "SPI init failed!\n"); 
		while(1); 
	}
	
	mpu6000_init(&mpu, spi, gpio, GPIO_PA4); 
	
	serial_flash_init(&flash, spi2, GPIO_PB12); 
	
	serial_printf(con, "Flash. ID: %x, Type: %x, Size: %x\n", flash.props.id, flash.props.type, flash.props.size); 
	
	pwm_configure(FC_PWM_CH1, 1500, 4000); 
	pwm_configure(FC_PWM_CH2, 1500, 4000); 
	pwm_configure(FC_PWM_CH3, 1500, 4000); 
	pwm_configure(FC_PWM_CH4, 1500, 4000); 
	pwm_configure(FC_PWM_CH5, 1500, 4000); 
	pwm_configure(FC_PWM_CH6, 1500, 4000); 
	
	int data = 0xbadfeed; 
	
	serial_printf(con, "Erasing flash..\n"); 
	serial_flash_sector_erase(&flash, 0); 
	
	serial_printf(con, "Writing value to flash: %x\n", data); 
	serial_flash_write(&flash, 0, (uint8_t*)&data, sizeof(data)); 
	
	while(1){
		gpio_set(GPIO_PB3); 
		
		int16_t ax, ay, az, gx, gy, gz; 
		mpu6000_getRawData(&mpu, &ax, &ay, &az, &gx, &gy, &gz); 
		
		int data = 0; 
		serial_flash_read(&flash, 0, (uint8_t*)&data, sizeof(data)); 
		
		serial_printf(con, "DATA: %x\n", data); 
		
		if(!spi){
			serial_printf(con, "No SPI! \n"); 
			continue; 
		}
		
		serial_printf(con, "Time: %d\n", timestamp_now()); 
		serial_printf(con, "Gyro: %d %d %d, Acc: %d %d %d\n", ax, ay, az, gx, gy, gz); 
		
		delay_ms(1); 
		gpio_clear(GPIO_PB3); 
		delay_ms(1); 
		//mpu6050_probe(&mpu); 
	}
}

void cc3d_process_events(void){
	
}

fc_board_t cc3d_get_fc_quad_interface(void){
	static fc_board_t _brd; 
	//static fc_board_t *brd = &_brd; 
	return _brd; 
}
