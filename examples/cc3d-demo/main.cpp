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
	
	while(1){
		printf("RC1: %d ", (int)pwm_read(FC_PWM_RC1)); 
		printf("RC2: %d ", (int)pwm_read(FC_PWM_RC2)); 
		printf("RC3: %d ", (int)pwm_read(FC_PWM_RC3)); 
		printf("RC4: %d ", (int)pwm_read(FC_PWM_RC4)); 
		printf("RC5: %d ", (int)pwm_read(FC_PWM_RC5)); 
		printf("RC6: %d ", (int)pwm_read(FC_PWM_RC6)); 
		
		struct fc_data s; 
		cc3d_read_sensors(&s); 
		
		printf("Time: %ld ", (long int)timestamp_now()); 
		printf("Gyro: %d %d %d, Acc: %d %d %d\n", 
			(int)(s.acc_g.x * 1000), (int)(s.acc_g.y * 1000), (int)(s.acc_g.z * 1000), 
			(int)(s.gyr_deg.x * 1000), (int)(s.gyr_deg.y * 1000), (int)(s.gyr_deg.z * 1000)); 
		
		serial_printf(flexiport, "Hello World!\n"); 
		
		if(timestamp_expired(ts)){
			led_state = ~led_state; 
			if(led_state) cc3d_led_on(); 
			else cc3d_led_off(); 
			ts = timestamp_from_now_us(500000); 
		}
	}
	
}
