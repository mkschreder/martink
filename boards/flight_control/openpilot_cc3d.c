#include "openpilot_cc3d.h"
#include <sensors/mpu6000.h>

#include <kernel.h>

void cc3d_init(void){
	time_init(); 
	timestamp_init(); 
	gpio_init(); 
	uart_init(); 
	twi_init(); 
	spi_init(); 
	
	struct mpu6000 mpu; 
	
	i2c_dev_t i2c = twi_get_interface(0); 
	serial_dev_t con = uart_get_serial_interface(0); 
	pio_dev_t gpio = gpio_get_parallel_interface(); 
	
	serial_printf(con, "Initializing sensor..\n"); 
	
	serial_dev_t spi = spi_get_serial_interface(0); 

	mpu6000_init(&mpu, spi, gpio, GPIO_PA4); 
	
	gpio_configure(GPIO_PB13, GP_OUTPUT); 
	
	while(1){
		gpio_set(GPIO_PB13); 
		
		int16_t ax, ay, az, gx, gy, gz; 
		mpu6000_getRawData(&mpu, &ax, &ay, &az, &gx, &gy, &gz); 
		
		if(!spi){
			serial_printf(con, "No SPI! \n"); 
			continue; 
		}
		
		serial_printf(con, "Time: %d\n", timestamp_now()); 
		serial_printf(con, "Gyro: %d %d %d, Acc: %d %d %d\n", ax, ay, az, gx, gy, gz); 
		
		gpio_clear(GPIO_PB13); 
		
		//delay_ms(1000); 
		//mpu6050_probe(&mpu); 
	}
}

void cc3d_process_events(void){
	
}

fc_board_t cc3d_get_fc_quad_interface(void){
	static fc_board_t _brd; 
	static fc_board_t *brd = &_brd; 
	return &_brd; 
}
