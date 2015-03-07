#include <arch/soc.h>
#include <disp/ks0713.h>
#include <tty/vt100.h>
#include <block/at24.h>
#include <hid/keymatrix.h>
#include <hid/fst6_keys.h>

#include "flysky-t6-tx.h"

#define KS0713_BACKLIGHT_PIN GPIO_PD2


#define FST6_ADC_BATTERY 6

#define FST6_PWM_SPEAKER PWM_CH11

#define FST6_STATUS_WR_CONFIG (1 << 0)
#define FST6_STATUS_RD_CONFIG (1 << 1)
#define FST6_STATUS_BEEP (1 << 2)

struct fst6_config {
	uint8_t *data; 
	uint16_t size; 
}; 

struct fst6 {
	struct ks0713 disp;
	struct vt100 vt; 
	struct at24 eeprom; 
	struct fst6_keys keyb; 
	timestamp_t time_to_render; 
	timestamp_t pwm_end_time; 
	uint16_t ppm_buffer[7]; 
	
	uint8_t  status; 
}; 

static void _fst6_write_ks0713(struct ks0713 *self, uint16_t *data, size_t size){
	(void)(self); 
	// special handling for backlight. We check only first byte to save time
	// backlight is connected to PD2 on flysky transmitter board
	if(size && *data | KS0713_BACKLIGHT) gpio_set(KS0713_BACKLIGHT_PIN); 
	else gpio_clear(KS0713_BACKLIGHT_PIN); 
	while(size--){
		GPIOC->ODR = (*data++ & KS0713_PIN_MASK); 
	}
}

static struct fst6 _board; 

int16_t fst6_read_key(void){
	return fst6_keys_get(&_board.keyb); 
}

uint8_t fst6_key_down(fst6_key_code_t key){
	return fst6_keys_key_down(&_board.keyb, key); 
}

uint16_t fst6_read_stick(fst6_stick_t id){
	if(id > FST6_STICKS_COUNT) return 0; 
	uint16_t val; 
	adc_start_read(id, &val); 
	return val; 
}

uint16_t fst6_read_battery_voltage(void){
	uint16_t val; 
	adc_start_read(FST6_ADC_BATTERY, &val); 
	return val; 
}

void fst6_play_tone(uint32_t freq, uint32_t duration_ms){
	// exit if invalid frequency or if another tone is in progress
	if(freq == 0 || !timestamp_expired(_board.pwm_end_time)) return; 
	//uint32_t period = 1000000UL / freq; 
	//pwm_set_period(FST6_PWM_SPEAKER, period); 
	//pwm_write(FST6_PWM_SPEAKER, period >> 2); 
	_board.pwm_end_time = timestamp_from_now_us(duration_ms * 1000UL); 
}

void fst6_write_ppm(uint16_t ch1, uint16_t ch2, uint16_t ch3, 
	uint16_t ch4, uint16_t ch5, uint16_t ch6){
	_board.ppm_buffer[0] = ch1; 
	_board.ppm_buffer[1] = ch2; 
	_board.ppm_buffer[2] = ch3; 
	_board.ppm_buffer[3] = ch4; 
	_board.ppm_buffer[4] = ch5; 
	_board.ppm_buffer[5] = ch6; 
	_board.ppm_buffer[6] = 20000 - ch1 - ch2 - ch3 - ch4 - ch5 - ch6; 
}

void fst6_init(void){
	//time_init(); 
	_board.pwm_end_time = 0; 
	_board.time_to_render = timestamp_now(); 
	
	timestamp_init(); 
	gpio_init(); 
	uart_init(0, 38400); 
	
	printf("FST6 Init\n"); 
	
	GPIO_InitTypeDef gpioInit;
	
	// Configure the LCD pins.
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = KS0713_PIN_MASK;
	GPIO_Init(GPIOC, &gpioInit);

	gpio_configure(KS0713_BACKLIGHT_PIN, GP_OUTPUT | GP_PUSH_PULL); 
	gpio_set(KS0713_BACKLIGHT_PIN); 
	
	fst6_keys_init(&_board.keyb); 
	
	gpio_configure(GPIO_PA0, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA1, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA2, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA3, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA4, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA5, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA6, GP_INPUT | GP_ANALOG); 
	
	printf("FST6: starting adc\n"); 
	adc_init(); 
	
	printf("FST6: starting i2c\n"); 
	i2cdev_init(0); 
	
	//delay_ms(1000); 
	
	printf("FST6: starting eeprom\n"); 
	at24_init(&_board.eeprom, twi_get_interface(0)); 
	
	printf("FST6: starting display\n"); 
	ks0713_init(&_board.disp, _fst6_write_ks0713); 
	
	printf("FST6: starting terminal\n"); 
	tty_dev_t tty = ks0713_get_tty_interface(&_board.disp); 
	vt100_init(&_board.vt, tty); 
	
	printf("FST6: starting ppm\n"); 
	for(int c = 0; c < 7; c++) _board.ppm_buffer[c] = 1000; 
	/*
	uint8_t buf[10]; 
	at24_read(&_board.eeprom, 0, buf, 10); 
	for(int c = 0; c < 10; c++) printf("%c ", buf[c]); 
	printf("\n");
	at24_write(&_board.eeprom, 0, (const uint8_t*)"0000000000", 10);  
	at24_write(&_board.eeprom, 5, (const uint8_t*)"12345", 5); 
	at24_read(&_board.eeprom, 0, buf, 10); 
	for(int c = 0; c < 10; c++) printf("%c ", buf[c]); 
	printf("\n"); 
	at24_write(&_board.eeprom, 0, (const uint8_t*)"12345", 5); 
	at24_read(&_board.eeprom, 0, buf, 10); 
	for(int c = 0; c < 10; c++) printf("%c ", buf[c]); 
	printf("\n"); */
	//static uint16_t ppm[6] = {1000, 1000, 2000, 2000, 5000, 5000}; 
	/*static uint16_t length[7]; 
	uint16_t acc = 0; 
	for(int c = 0; c < 6; c++){
		acc += ppm[c]; 
		length[c] = acc; 
	}
	length[6] = 20000 - acc; */
	//ppm_configure(PWM_CH14, 0, 0, ppm, 6); 
	ppm_configure(PWM_CH14, _board.ppm_buffer, 7, 400); 
}
/*
static PT_THREAD(_fst6_eeprom_thread(struct pt *thr)){
	PT_BEGIN(thr); 
	
	if(_board.status & FST6_STATUS_WR_CONFIG){
		
	} else if(_board.status & FST6_STATUS_RD_CONFIG){
		
	}
	
	PT_END(thr); 
}*/

LIBK_THREAD(_speaker_silent){
	static timestamp_t time = 0; 
	PT_BEGIN(pt); 
	
	while(1){
		PT_WAIT_UNTIL(pt, _board.status & FST6_STATUS_BEEP); 
		// do the beep 
		time = timestamp_from_now_us(200000); 
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
	}
	
	PT_END(pt); 
}
/*
LIBK_THREAD(_fst6_ks0713_commit){
	static timestamp_t time = 0; 
	PT_BEGIN(pt); 
	while(1){
		PT_WAIT_UNTIL(pt, timestamp_expired(time)); 
		ks0713_commit(&_board.disp); 
		time = timestamp_from_now_us(20000); 
	}
	PT_END(pt); 
}
*/
/*
int8_t fst6_write_config(const uint8_t *data, uint16_t size){
	(void)(data); 
	(void)(size); 
	//at24_start_write(&_board.eeprom, 0, data, size); 
	return 0; 
}

int8_t fst6_read_config(uint8_t *data, uint16_t size){
	(void)(data); 
	(void)(size); 
	//at24_start_read(&_board.eeprom, 0, data, size); 
	return 0; 
}
*/
block_dev_t fst6_get_storage_device(void){
	return at24_get_block_device_interface(&_board.eeprom); 
}

serial_dev_t fst6_get_screen_serial_interface(void){
	return vt100_get_serial_interface(&_board.vt); 
}

fbuf_dev_t fst6_get_screen_framebuffer_interface(void){
	return ks0713_get_framebuffer_interface(&_board.disp); 
}
