/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/flight_control/openpilot_cc3d.h>
#include <tty/vt100.h>

extern char _sdata; 

int main(void){
	cc3d_init(); 
	
	printf("SystemCoreClock: %d\n", (int)SystemCoreClock); 
	
	// test config read/write (to eeprom)
	const char str[] = "Hello World!"; 
	uint8_t buf[13] = {0}; 
	printf("Writing string to config: %s\n", str); 
	cc3d_write_config((const uint8_t*)str, sizeof(str)); 
	printf("Reading string from config: "); 
	cc3d_read_config(buf, sizeof(str)); 
	printf("%s\n", buf); 
	
	uint8_t led_state = 0; 
	timestamp_t ts = timestamp_from_now_us(500000); 
	
	serial_dev_t flexiport = cc3d_get_flexiport_serial_interface(); 
	
	unsigned int loop = 0; 
	
	while(1){
		printf("RC1: %d ", (int)cc3d_read_pwm(CC3D_IN_PWM1)); 
		printf("RC2: %d ", (int)cc3d_read_pwm(CC3D_IN_PWM2)); 
		printf("RC3: %d ", (int)cc3d_read_pwm(CC3D_IN_PWM3)); 
		printf("RC4: %d ", (int)cc3d_read_pwm(CC3D_IN_PWM4)); 
		printf("RC5: %d ", (int)cc3d_read_pwm(CC3D_IN_PWM5)); 
		printf("RC6: %d ", (int)cc3d_read_pwm(CC3D_IN_PWM6)); 
		
		cc3d_write_pwm(CC3D_OUT_PWM1, 1000 + loop % 1000); 
		cc3d_write_pwm(CC3D_OUT_PWM2, 1000 + loop % 1000); 
		cc3d_write_pwm(CC3D_OUT_PWM3, 1000 + loop % 1000); 
		cc3d_write_pwm(CC3D_OUT_PWM4, 1000 + loop % 1000); 
		cc3d_write_pwm(CC3D_OUT_PWM5, 1000 + loop % 1000); 
		cc3d_write_pwm(CC3D_OUT_PWM6, 1000 + loop % 1000); 
		
		float ax, ay, az, gx, gy, gz; 
		cc3d_read_acceleration_g(&ax, &ay, &az); 
		cc3d_read_angular_velocity_dps(&gx, &gy, &gz); 
		
		printf("Time: %ld ", (long int)timestamp_now()); 
		printf("Gyro: %d %d %d, Acc: %d %d %d\n", 
			(int)(ax * 1000), (int)(ay * 1000), (int)(az * 1000), 
			(int)(gx * 1000), (int)(gy * 1000), (int)(gz * 1000)); 
		
		serial_printf(flexiport, "Hello World!\n"); 
		
		if(timestamp_expired(ts)){
			led_state = ~led_state; 
			if(led_state) cc3d_led_on(); 
			else cc3d_led_off(); 
			ts = timestamp_from_now_us(500000); 
			
			loop += 100; 
		}
		
	}
	
}
