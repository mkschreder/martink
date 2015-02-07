#include "openpilot_cc3d.h"
#include <stdio.h>
#include <sensors/mpu6000.h>
#include <block/serial_flash.h>
#include <fs/cfs/cfs.h>

#include <kernel.h>

#define FC_PWM_CH1 PWM_CH44
#define FC_PWM_CH2 PWM_CH43
#define FC_PWM_CH3 PWM_CH42
#define FC_PWM_CH4 PWM_CH11
#define FC_PWM_CH5 PWM_CH31
#define FC_PWM_CH6 PWM_CH23

#define FC_PWM_RC1 PWM_CH41
#define FC_PWM_RC2 PWM_CH32
#define FC_PWM_RC3 PWM_CH33
#define FC_PWM_RC4 PWM_CH34
#define FC_PWM_RC5 PWM_CH21
#define FC_PWM_RC6 PWM_CH22

/*
static void test(void){
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

	GPIO_InitStructure.GPIO_Pin =
					GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	USART_InitTypeDef usartConfig;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 |
												 RCC_APB2Periph_GPIOA |
												 RCC_APB2Periph_GPIOB |
												 RCC_APB2Periph_AFIO, ENABLE);

	usartConfig.USART_BaudRate = 38400;
	usartConfig.USART_WordLength = USART_WordLength_8b;
	usartConfig.USART_StopBits = USART_StopBits_1;
	usartConfig.USART_Parity = USART_Parity_No;
	usartConfig.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	usartConfig.USART_HardwareFlowControl =
			 USART_HardwareFlowControl_None;
	USART_Init(USART1, &usartConfig);
	USART_Cmd(USART1, ENABLE);

	GPIO_InitTypeDef gpioConfig;

	//PA9 = USART1.TX => Alternative Function Output
	gpioConfig.GPIO_Mode = GPIO_Mode_AF_PP;
	gpioConfig.GPIO_Pin = GPIO_Pin_9;
	gpioConfig.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &gpioConfig);

	//PA10 = USART1.RX => Input
	gpioConfig.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpioConfig.GPIO_Pin = GPIO_Pin_10;
	GPIO_Init(GPIOA, &gpioConfig);
	
	for (;;)
	{
			USART_SendData(USART1, 0x55);
			while(USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);  // Wait for Empty
	}
}

void _sbrk(void){
	while(1); 
}
*/
#define led_on() gpio_clear(GPIO_PB3)
#define led_off() gpio_set(GPIO_PB3)

struct serial_flash flash; 
	
void stm32w_flash_read(uint32_t address, void *data, uint32_t length){
	//serial_dev_t con = uart_get_serial_interface(0); 
	//printf("READ: @%x -> @%p\n", address, data); 
	serial_flash_read(&flash, address, (uint8_t*)data, length); 
}

void stm32w_flash_write(uint32_t address, const void *data, uint32_t length){
	//serial_dev_t con = uart_get_serial_interface(0); 
	printf("WRITE: @%x -> @%p %d\n", (int)address, data, (int)length); 
	serial_flash_write(&flash, (int)address, (uint8_t*)data, (int)length); 
}

void stm32w_flash_erase(uint8_t sector){
	//serial_dev_t con = uart_get_serial_interface(0); 
	printf("ERASE: @%x\n", (int)sector); 
	serial_flash_sector_erase(&flash, sector);
}

void cc3d_init(void){
	time_init(); 
	timestamp_init(); 
	gpio_init(); 
	
	led_off(); 
	gpio_configure(GPIO_PB3, GP_OUTPUT); 
	  
	uart_init(); 
	//twi_init(); 
	spi_init(); 
	
	led_on(); 
	delay_ms(500); 
	led_off(); 
	delay_ms(500); 
	led_on(); 
	delay_ms(500); 
	led_off(); 
	
	struct mpu6000 mpu; 
	
	//i2c_dev_t i2c = twi_get_interface(0); 
	//serial_dev_t con = uart_get_serial_interface(0); 
	pio_dev_t gpio = gpio_get_parallel_interface(); 
	
	printf("Initializing sensor..\n"); 
	
	serial_dev_t spi = spi_get_serial_interface(0); 
	serial_dev_t spi2 = spi_get_serial_interface(1); 
	
	if(!spi || !spi2) {
		printf("SPI init failed!\n"); 
		while(1); 
	}
	
	mpu6000_init(&mpu, spi, gpio, GPIO_PA4); 
	
	serial_flash_init(&flash, spi2, GPIO_PB12); 
	
	printf("Flash. ID: %x, Type: %x, Size: %x\n", flash.props.id, flash.props.type, flash.props.size); 
	
	pwm_configure(FC_PWM_CH1, 1250, 4000); 
	pwm_configure(FC_PWM_CH2, 1250, 4000); 
	pwm_configure(FC_PWM_CH3, 1250, 4000); 
	pwm_configure(FC_PWM_CH4, 1250, 4000); 
	pwm_configure(FC_PWM_CH5, 1250, 4000); 
	pwm_configure(FC_PWM_CH6, 1250, 4000); 
	
	pwm_configure_capture(FC_PWM_RC1, 1000); 
	pwm_configure_capture(FC_PWM_RC2, 1000); 
	pwm_configure_capture(FC_PWM_RC3, 1000); 
	pwm_configure_capture(FC_PWM_RC4, 1000); 
	pwm_configure_capture(FC_PWM_RC5, 1000); 
	pwm_configure_capture(FC_PWM_RC6, 1000); 
	
	/*
	{
		int fd = cfs_open("test", CFS_READ);
		char message[5] = {0}; 
		if(fd != -1) {
			cfs_read(fd, message, sizeof(message));
			cfs_close(fd);
			printf("READ: %s\n", message); 
		} else {
			printf("ERROR: could not open file for reading!\n");
		}
	}
	
	{
		int fd_write = cfs_open("test", CFS_WRITE);
		char message[5] = "alfa"; 
		if(fd_write != -1) {
			cfs_write(fd_write, message, sizeof(message));
			cfs_close(fd_write);
			printf("WRITE: %s\n", message); 
		} else {
			printf("ERROR: could not write to memory in step 2.\n");
		}
	}
	
	{
		int fd = cfs_open("test", CFS_READ);
		char message[5] = {0}; 
		if(fd != -1) {
			cfs_read(fd, message, sizeof(message));
			cfs_close(fd);
			printf("READ: %s\n", message); 
		} else {
			printf("ERROR: could not open file for reading.\n");
		}
	}
	
	cfs_remove("test");
	
	int fd_write = cfs_open("test", CFS_WRITE);
	char message[5] = "foob"; 
	if(fd_write != -1) {
		cfs_write(fd_write, message, sizeof(message));
		cfs_close(fd_write);
		printf("WRITE: %s\n", message); 
	} else {
		printf("ERROR: could not write to memory in step 2.\n");
	}

	{
		int fd = cfs_open("test", CFS_READ);
		char message[5] = {0}; 
		if(fd != -1) {
			cfs_read(fd, message, sizeof(message));
			cfs_close(fd);
			printf("READ: %s\n", message); 
		} else {
			printf("ERROR: could not write to memory in step 2.\n");
		}
	}
*/
	while(1){
		gpio_set(GPIO_PB3); 
		
		printf("RC1: %d ", (int)pwm_read(FC_PWM_RC1)); 
		printf("RC2: %d ", (int)pwm_read(FC_PWM_RC2)); 
		printf("RC3: %d\n", (int)pwm_read(FC_PWM_RC3)); 
		printf("RC4: %d ", (int)pwm_read(FC_PWM_RC4)); 
		printf("RC5: %d ", (int)pwm_read(FC_PWM_RC5)); 
		printf("RC6: %d\n", (int)pwm_read(FC_PWM_RC6)); 
		
		int16_t ax, ay, az, gx, gy, gz; 
		mpu6000_getRawData(&mpu, &ax, &ay, &az, &gx, &gy, &gz); 
		
		//int data = 0; 
		///serial_flash_read(&flash, 0, (uint8_t*)&data, sizeof(data)); 
		
		//printf("DATA: %x\n", data); 
		
		if(!spi){
			printf("No SPI! \n"); 
			continue; 
		}
		
		printf("Time: %ld\n", (long int)timestamp_now()); 
		printf("Gyro: %d %d %d, Acc: %d %d %d\n", ax, ay, az, gx, gy, gz); 
		
		delay_ms(1); 
		gpio_clear(GPIO_PB3); 
		delay_ms(1); 
		//mpu6050_probe(&mpu); 
	}
}

void cc3d_process_events(void){
	
}

fc_board_t cc3d_get_fc_quad_interface(void){
	static fc_board_t _brd; 
	//static fc_board_t *brd = &_brd; 
	return _brd; 
}
