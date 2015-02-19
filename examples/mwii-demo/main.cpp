/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/flight_control/multiwii.h>
#include <tty/vt100.h>
#include <util/list.h>
#include <util/cbuf.h>
#include <util/pipe.h>

#include <thread/pt.h>

#define typeof __typeof__

struct application {
	struct pt thread; 
	serial_dev_t uart; 
	uint16_t adc; 
	timestamp_t time; 
	struct pin_state state; 
}; 

static struct application app; 
/*
int8_t _on_adc_event(struct adc_connection *con, uint16_t ev){
	struct application *app = container_of(con, struct application, con_adc); 
	switch(ev){
		case ADC_EV_CONV_COMPLETED: {
			timestamp_t time; 
			uint8_t chan; 
			uint16_t value; 
			if(adc_read_ev_conv_completed(con, &time, &chan, &value) == 0){
				app->adc[chan & 0x07] = value; 
				return 0; 
			}
		} break; 
	}
	return -1; 
}
*/
PT_THREAD(app_thread(struct application *app)){
	PT_BEGIN(&app->thread); 
	while(1){
		uint16_t ch; 
		PT_WAIT_WHILE(&app->thread, (ch = serial_getc(app->uart)) == SERIAL_NO_DATA); 
		app->time = timestamp_now(); 
		printf("Measuring adc.. \n"); 
		adc_start_read(1, &app->adc); 
		PT_WAIT_WHILE(&app->thread, adc_busy()); 
		printf("Measured to %u in %lu us\n", app->adc, (uint32_t)timestamp_ticks_to_us((timestamp_now() - app->time))); 
		printf("Reading N pulse..\n"); 
		gpio_start_read(MWII_GPIO_D12, &app->state, GP_READ_PULSE_P); 
		PT_WAIT_WHILE(&app->thread, gpio_pin_busy(MWII_GPIO_D12)); 
		printf("Pulse length: %lu\n", timestamp_ticks_to_us((app->state.t_down - app->state.t_up))); 
		float ax, ay, az; 
		timestamp_t t = timestamp_now(); 
		mwii_read_acceleration_g(&ax, &ay, &az); 
		t = timestamp_ticks_to_us(timestamp_now() - t); 
		printf("ACC: %d %d %d, time: %lu us\n", 
			(int16_t)(ax * 1000), (int16_t)(ay * 1000), (int16_t)(az * 1000), (uint32_t)t);  
		//app->time = timestamp_from_now_us(1000000); 
		//PT_WAIT_UNTIL(&app->thread, timestamp_expired(app->time)); 
	}
	PT_END(&app->thread); 
}

int main(void){
	mwii_init(); 
	
	PT_INIT(&app.thread); 
	app.uart = mwii_get_uart_interface(); 
	
	gpio_configure(MWII_GPIO_A1, GP_INPUT | GP_PULLUP | GP_ANALOG); 
	
	while(1){
		app_thread(&app); 
		mwii_process_events(); 
		/*
		static timestamp_t t = 0; 
		if(timestamp_expired(t)){
			printf("ADC: "); 
			for(int c = 0; c < 8; c++){
				printf("%u ", app.adc[c]); 
			}
			printf("\n"); 
			t = timestamp_from_now_us(1000000UL);  
		}
		adc_process_events(); */
		/*struct adc_msg_value msg; 
		//struct adc_msg_value in = {0, 3, 1234}; 
		//adc_msg_value_pack(&ac, &in); 
		if(adc_msg_value_parse(&ac, &msg) != -1){
			printf("\nADC: %d %u ", msg.chan, msg.val); 
		} else {
			//printf("."); 
		}*/
	}
	
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
		/*
		printf("A0: %5u, A1: %5u, A2: %5u\n", 
			mwii_read_adc(MWII_GPIO_A0), 
			mwii_read_adc(MWII_GPIO_A1), 
			mwii_read_adc(MWII_GPIO_A2)); 
		*/
		if(timestamp_expired(ts)){
			led_state = ~led_state; 
			if(led_state) mwii_led_on(); 
			else mwii_led_off(); 
			ts = timestamp_from_now_us(500000); 
		}
	}
}
