/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/flight_control/multiwii.h>
#include <tty/vt100.h>

extern char _sdata; 

int main(void){
	mwii_init(); 

	// test config read/write (to eeprom)
	const char str[] = "Hello World!"; 
	uint8_t buf[13] = {0}; 
	printf("Writing string to config: %s\n", str); 
	mwii_write_config((const uint8_t*)str, sizeof(str)); 
	printf("Reading string from config: "); 
	mwii_read_config(buf, sizeof(str)); 
	printf("%s\n", buf); 
	
	uint8_t led_state = 0; 
	timestamp_t ts = timestamp_from_now_us(500000); 
	
	while(1){
		printf("RC1: %d ", (int)mwii_read_pwm(MWII_IN_PWM0)); 
		printf("RC2: %d ", (int)mwii_read_pwm(MWII_IN_PWM1)); 
		printf("RC3: %d ", (int)mwii_read_pwm(MWII_IN_PWM2)); 
		printf("RC4: %d ", (int)mwii_read_pwm(MWII_IN_PWM3)); 
		
		float ax, ay, az, gx, gy, gz, temp, pres; 
		mwii_read_acceleration_g(&ax, &ay, &az); 
		mwii_read_angular_velocity_dps(&gx, &gy, &gz); 
		pres = mwii_read_pressure_pa(); 
		temp = mwii_read_temperature_c(); 
		
		printf("Time: %ld ", (long int)timestamp_ticks_to_us(timestamp_now())); 
		printf("Temp: %d, Pres: %d, Gyro: %d %d %d, Acc: %d %d %d\n", 
			(int)temp, (int)pres, (int)(ax * 1000), (int)(ay * 1000), (int)(az * 1000), 
			(int)(gx * 1000), (int)(gy * 1000), (int)(gz * 1000)); 
	
		if(timestamp_expired(ts)){
			led_state = ~led_state; 
			if(led_state) mwii_led_on(); 
			else mwii_led_off(); 
			ts = timestamp_from_now_us(500000); 
		}
	}
}
