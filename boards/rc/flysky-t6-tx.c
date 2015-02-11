#include <arch/soc.h>
#include <disp/ks0713.h>
#include <tty/vt100.h>

#include "flysky-t6-tx.h"

#define KS0713_PIN_MASK 0x1fffUL
#define KS0713_BACKLIGHT_PIN (1 << 2)

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

struct fst6 {
	struct ks0713 disp;
	struct vt100 vt; 
	uint32_t keys; 
}; 

static void _fst6_write_ks0713(struct ks0713 *self, uint16_t byte){
	(void)(self); 
	GPIO_Write(GPIOC, byte & KS0713_PIN_MASK);
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

void fst6_init(void){
	//time_init(); 
	
	timestamp_init(); 
	
	gpio_init(); 
	
	GPIO_InitTypeDef gpioInit;
	
	// Configure the LCD pins.
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	gpioInit.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInit.GPIO_Pin = KS0713_PIN_MASK;
	GPIO_Init(GPIOC, &gpioInit);

	gpioInit.GPIO_Pin = KS0713_BACKLIGHT_PIN;
	GPIO_Init(GPIOD, &gpioInit);
	
	GPIO_SetBits(GPIOD, KS0713_BACKLIGHT_PIN); 
	
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
	
	uart_init(); 
	adc_init(); 
	
	ks0713_init(&_board.disp, _fst6_write_ks0713); 
	tty_dev_t tty = ks0713_get_tty_interface(&_board.disp); 
	vt100_init(&_board.vt, tty); 
	
	/*
	vt100_puts(&vt, "\x1b[A\x1b[KHello World! This is FlySky FS-T6 transmitter. We are currently testing the terminal! We are printing text that goes on the screen and looking at how it all works out!"); 
	
	while(1){
		for(int c = 0; c < 6; c++) {
			printf("ADC%d: %d\n", c, (int)adc_read(c)); 
		}
		
		_fst6_scan_keys(&_board); 
	
		printf("KEYS: "); 
		for(int c = 0; c < 32; c++){
			if(_board.keys & (1 << c)){
				printf("%d ", c); 
			}
		}
		printf("\n"); 
		//ks0713_draw_line(&disp, 1, 1, 10, 10, KS0713_OP_SET); 
		ks0713_commit(&disp); 
	}*/
}

void fst6_process_events(void){
	ks0713_commit(&_board.disp); 
}

serial_dev_t fst6_get_screen_serial_interface(void){
	return vt100_get_serial_interface(&_board.vt); 
}
