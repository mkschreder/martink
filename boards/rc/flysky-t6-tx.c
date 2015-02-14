#include <arch/soc.h>
#include <disp/ks0713.h>
#include <tty/vt100.h>
#include <block/at24.h>

#include "flysky-t6-tx.h"

#define KS0713_BACKLIGHT_PIN GPIO_PD2

#define KEYS_COL1 GPIO_PB8
#define KEYS_COL2 GPIO_PB9
#define KEYS_COL3 GPIO_PB10
#define KEYS_COL4 GPIO_PB11
#define KEYS_ROW1 GPIO_PB12
#define KEYS_ROW2 GPIO_PB13
#define KEYS_ROW3 GPIO_PB14
#define KEYS_ROTA GPIO_PC14
#define KEYS_ROTB GPIO_PC15
#define KEYS_SWA GPIO_PB0
#define KEYS_SWB GPIO_PB1
#define KEYS_SWC GPIO_PB5
#define KEYS_SWD GPIO_PC13

#define FST6_ADC_BATTERY 6

#define FST6_PWM_SPEAKER PWM_CH11

struct fst6 {
	struct ks0713 disp;
	struct vt100 vt; 
	struct at24 eeprom; 
	timestamp_t time_to_render; 
	timestamp_t pwm_end_time; 
	uint32_t keys; 
	uint16_t ppm_buffer[7]; 
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

fst6_key_mask_t fst6_read_keys(void){
	fst6_key_mask_t keys = 0; 
	
	gpio_set(KEYS_COL1); 
	gpio_set(KEYS_COL2); 
	gpio_set(KEYS_COL3); 
	gpio_set(KEYS_COL4); 
	keys = 0; 
	gpio_clear(KEYS_COL1); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= FST6_KEY_CH1P; 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= FST6_KEY_CH3P; 
	gpio_set(KEYS_COL1); 
	gpio_clear(KEYS_COL2); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= FST6_KEY_CH1M; 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= FST6_KEY_CH3M; 
	if(!gpio_read_pin(KEYS_ROW3)) keys |= FST6_KEY_SELECT; 
	gpio_set(KEYS_COL2); 
	gpio_clear(KEYS_COL3); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= FST6_KEY_CH2P; 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= FST6_KEY_CH4P; 
	if(!gpio_read_pin(KEYS_ROW3)) keys |= FST6_KEY_OK; 
	gpio_set(KEYS_COL3); 
	gpio_clear(KEYS_COL4); 
	//delay_us(5); 
	if(!gpio_read_pin(KEYS_ROW1)) keys |= FST6_KEY_CH2M; 
	if(!gpio_read_pin(KEYS_ROW2)) keys |= FST6_KEY_CH4M; 
	if(!gpio_read_pin(KEYS_ROW3)) keys |= FST6_KEY_CANCEL; 
	gpio_set(KEYS_COL4);
	if(!gpio_read_pin(KEYS_ROTA)) keys |= FST6_KEY_ROTA; 
	if(!gpio_read_pin(KEYS_ROTB)) keys |= FST6_KEY_ROTB; 
	if(!gpio_read_pin(KEYS_SWA)) keys |= FST6_KEY_SWA; 
	if(!gpio_read_pin(KEYS_SWB)) keys |= FST6_KEY_SWB; 
	if(!gpio_read_pin(KEYS_SWC)) keys |= FST6_KEY_SWC; 
	if(!gpio_read_pin(KEYS_SWD)) keys |= FST6_KEY_SWD; 
	
	return keys; 
}

uint16_t fst6_read_stick(fst6_stick_t id){
	if(id > FST6_STICKS_COUNT) return 0; 
	return adc_read(id); 
}

uint16_t fst6_read_battery_voltage(void){
	return adc_read(FST6_ADC_BATTERY); 
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
	uart_init(); 
	
	GPIO_InitTypeDef gpioInit;
	
	// Configure the LCD pins.
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = KS0713_PIN_MASK;
	GPIO_Init(GPIOC, &gpioInit);

	gpio_configure(KS0713_BACKLIGHT_PIN, GP_OUTPUT | GP_PUSH_PULL); 
	gpio_set(KS0713_BACKLIGHT_PIN); 
	
	// configure pins for keys
	gpio_configure(KEYS_COL1, GP_OUTPUT); 
	gpio_configure(KEYS_COL2, GP_OUTPUT); 
	gpio_configure(KEYS_COL3, GP_OUTPUT); 
	gpio_configure(KEYS_COL4, GP_OUTPUT); 
	
	gpio_configure(KEYS_ROW1, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROW2, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROW3, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROTA, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_ROTB, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWA, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWB, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWC, GP_INPUT | GP_PULLUP); 
	gpio_configure(KEYS_SWD, GP_INPUT | GP_PULLUP); 
	
	gpio_configure(GPIO_PA0, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA1, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA2, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA3, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA4, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA5, GP_INPUT | GP_ANALOG); 
	gpio_configure(GPIO_PA6, GP_INPUT | GP_ANALOG); 
	
	adc_init(); 
	
	twi0_init_default(); 
	
	//delay_ms(1000); 
	
	at24_init(&_board.eeprom, twi_get_interface(0)); 
	
	ks0713_init(&_board.disp, _fst6_write_ks0713); 
	tty_dev_t tty = ks0713_get_tty_interface(&_board.disp); 
	
	vt100_init(&_board.vt, tty); 
	
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

void fst6_process_events(void){
	// limit screen updates to 50fps
	if(timestamp_expired(_board.time_to_render)){
		ks0713_commit(&_board.disp); 
		_board.time_to_render = timestamp_from_now_us(20000); 
	}
	
	if(timestamp_expired(_board.pwm_end_time)){
		pwm_write(FST6_PWM_SPEAKER, 0); 
	}
}

int8_t fst6_write_config(const uint8_t *data, uint16_t size){
	(void)(data); 
	(void)(size); 
	at24_write(&_board.eeprom, 0, data, size); 
	return 0; 
}

int8_t fst6_read_config(uint8_t *data, uint16_t size){
	(void)(data); 
	(void)(size); 
	at24_read(&_board.eeprom, 0, data, size); 
	return 0; 
}

serial_dev_t fst6_get_screen_serial_interface(void){
	return vt100_get_serial_interface(&_board.vt); 
}
