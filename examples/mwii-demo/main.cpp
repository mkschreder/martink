/*
Example for the fst6 radio transmitter board 

*/ 

#include <kernel.h>
#include <boards/flight_control/multiwii.h>
#include <tty/vt100.h>
#include <kernel/list.h>
#include <kernel/cbuf.h>
//#include <kernel/pipe.h>

#include <kernel/thread.h>

#define typeof __typeof__

struct application {
	struct async_process process; 
	serial_dev_t uart; 
	timestamp_t time; 
	//struct pin_state state; 
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
ASYNC_PROCESS(app_task){
	//struct application *self = container_of(__self, struct application, process); 
	ASYNC_BEGIN(); 
	while(1){
		
		static timestamp_t time = 0; 
		static uint32_t fps = 0; 
		static uint32_t frame_time; 
		static timestamp_t t = 0; 
		
		t = timestamp_from_now_us(10000);
		
		//uint16_t ch; 
		/*PT_WAIT_WHILE(pt, (ch = serial_getc(app->uart)) == SERIAL_NO_DATA); 
		app->time = timestamp_now(); 
		printf("Measuring adc.. \n"); 
		adc_start_read(1, &app->adc); 
		PT_WAIT_WHILE(pt, adc_busy()); 
		printf("Measured to %u in %lu us\n", app->adc, (uint32_t)timestamp_ticks_to_us((timestamp_now() - app->time))); 
		printf("Reading N pulse..\n"); 
		gpio_start_read(MWII_GPIO_D9, &app->state, GP_READ_PULSE_P); 
		PT_WAIT_WHILE(pt, gpio_pin_busy(MWII_GPIO_D9)); 
		printf("Pulse length: %lu\n", timestamp_ticks_to_us((app->state.t_down - app->state.t_up))); 
		*/
		float ax, ay, az, gx, gy, gz, x, y, z; 
		//timestamp_t t = timestamp_now(); 
		mwii_read_acceleration_g(&ax, &ay, &az); 
		mwii_read_angular_velocity_dps(&gx, &gy, &gz); 
		mwii_read_magnetic_field(&x, &y, &z); 
		mwii_read_temperature_c();
		mwii_read_pressure_pa(); 
		
		if(timestamp_expired(time)){
			time = timestamp_from_now_us(1000000); 
			printf("ACC: %5d %5d %5d, ", 
				(int16_t)(ax * 1000), (int16_t)(ay * 1000), (int16_t)(az * 1000));  
			printf("GYR: %5d %5d %5d, ", 
				(int16_t)(gx * 1000), (int16_t)(gy * 1000), (int16_t)(gz * 1000));  
			printf("TEMP: %5d, PRES: %6lu ", (int16_t)(mwii_read_temperature_c() * 10), (long)mwii_read_pressure_pa()); 
			
			printf("MAG: %5d %5d %5d ", 
				(int16_t)(x * 10), (int16_t)(y * 10), (int16_t)(z * 10));  
			printf("FRAME: %6luus\n", (uint32_t)t); 
			printf("APP FPS: %5lu, %5luus\n", fps, frame_time); 
			fps = 0; frame_time = 0; 
		}
		
		AWAIT(timestamp_expired(t)); 
		t = timestamp_ticks_to_us(timestamp_now() - t); 
		frame_time += t; 
		
		fps++; 
	}
	ASYNC_END(0); 
}

int main(void){
	mwii_init(); 
	
	app.uart = mwii_get_uart_interface(); 
	
	gpio_configure(MWII_GPIO_A1, GP_INPUT | GP_PULLUP | GP_ANALOG); 
	gpio_configure(MWII_GPIO_D9, GP_INPUT | GP_PULLUP | GP_PCINT); 
	
	ASYNC_PROCESS_INIT(&app.process, app_task); 
	ASYNC_QUEUE_WORK(&ASYNC_GLOBAL_QUEUE, &app.process); 
	
	while(ASYNC_RUN_PARALLEL(&ASYNC_GLOBAL_QUEUE)); 
	//libk_run(); 
	/*while(1){
		app_thread(&app); 
		mwii_process_events(); 
		if(timestamp_expired(time)){
			printf("FPS: %lu\n", frames); 
			time = timestamp_from_now_us(1000000); 
			frames = 0; 
		}
		frames++; 
		
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
	*/
}
