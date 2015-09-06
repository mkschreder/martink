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

ASYNC_PROCESS(app_task){
	//struct application *self = container_of(__self, struct application, process); 
	ASYNC_BEGIN(); 
	while(1){
		static timestamp_t time = 0; 
		static uint32_t fps = 0; 
		static uint32_t frame_time; 
		static timestamp_t t = 0; 
		
		t = timestamp_from_now_us(10000);
		
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
}
