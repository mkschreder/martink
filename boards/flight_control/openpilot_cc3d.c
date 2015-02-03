#include "openpilot_cc3d.h"
#include <sensors/mpu6050.h>

#include <kernel.h>

void cc3d_init(void){
	uart_init(); 
	twi_init(); 
	spi_init(); 
	
	struct mpu6050 mpu; 
	
	i2c_dev_t i2c = twi_get_interface(0); 
	serial_dev_t con = uart_get_serial_interface(0); 
	
	serial_printf(con, "Initializing sensor..\n"); 
	
	serial_dev_t spi = spi_get_serial_interface(0); 

	//mpu6050_init(&mpu, i2c, MPU6050_ADDR); 
	
	static int foo = 123; 
	
	while(1){
		if(!spi){
			serial_printf(con, "No SPI! \n"); 
			continue; 
		}
		
		serial_printf(con, "Loop %d\n", foo); 
		serial_printf(spi, "Foobar!\n"); 
		serial_putc(spi, 0x51); 
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
