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
	
	// configure pins for analog input (with internal pullups)
	mwii_configure_pin(MWII_GPIO_A0, GP_INPUT | GP_ANALOG | GP_PULLUP); 
	mwii_configure_pin(MWII_GPIO_A1, GP_INPUT | GP_ANALOG | GP_PULLUP); 
	mwii_configure_pin(MWII_GPIO_A2, GP_INPUT | GP_ANALOG | GP_PULLUP); 
	
	while(1){
		mwii_process_events(); 
		
		printf("RC1: %d ", (int)mwii_read_pwm(MWII_IN_PWM_D2)); 
		printf("RC2: %d ", (int)mwii_read_pwm(MWII_IN_PWM_D4)); 
		printf("RC3: %d ", (int)mwii_read_pwm(MWII_IN_PWM_D7)); 
		printf("RC4: %d ", (int)mwii_read_pwm(MWII_IN_PWM_D8)); 
		
		float ax, ay, az, gx, gy, gz, temp, pres; 
		mwii_read_acceleration_g(&ax, &ay, &az); 
		mwii_read_angular_velocity_dps(&gx, &gy, &gz); 
		pres = mwii_read_pressure_pa(); 
		temp = mwii_read_temperature_c(); 
		
		mwii_write_pwm(MWII_OUT_PWM_D6, 800); 
		mwii_write_pwm(MWII_OUT_PWM_D5, 1200); 
		mwii_write_pwm(MWII_OUT_PWM_D11, 1500); 
		mwii_write_pwm(MWII_OUT_PWM_D3, 1800); 
		
		printf("Time: %ld ", (long int)timestamp_ticks_to_us(timestamp_now())); 
		printf("Temp: %u, Pres: %lu, Acc: %5d %5d %5d, Gyro: %5d %5d %5d, ", 
			(int16_t)(temp * 100), (uint32_t)pres, 
			(int)(ax * 1000), (int)(ay * 1000), (int)(az * 1000), 
			(int)(gx * 1000), (int)(gy * 1000), (int)(gz * 1000)); 
		
		// reading analog and digital pins
		
		printf("A0: %5u, A1: %5u, A2: %5u\n", 
			mwii_read_adc(MWII_GPIO_A0), 
			mwii_read_adc(MWII_GPIO_A1), 
			mwii_read_adc(MWII_GPIO_A2)); 
		
		if(timestamp_expired(ts)){
			led_state = ~led_state; 
			if(led_state) mwii_led_on(); 
			else mwii_led_off(); 
			ts = timestamp_from_now_us(500000); 
		}
	}
}
