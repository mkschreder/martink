#include "openpilot_cc3d.h"
#include <sensors/mpu6050.h>

#include <kernel.h>

void cc3d_init(void){
	uart_init(); 
	twi_init(); 
	
	struct mpu6050 mpu; 
	
	i2c_dev_t i2c = twi_get_interface(0); 
	serial_dev_t con = uart_get_serial_interface(0); 
	
	serial_printf(con, "Initializing sensor..\n"); 
	
	//mpu6050_init(&mpu, i2c, MPU6050_ADDR); 
	
	while(1){
		serial_printf(con, "Loop\n"); 
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
