#include <arch/soc.h>
#include "spi.h"

void spi0_init_default(void){
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi; 
	
	RCC_APB2PeriphClockCmd ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE); 
	
	// Configure SCK and MOSI 
	gpio.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpio);

	// Configure MISO
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin = GPIO_Pin_6;
	GPIO_Init(GPIOA, &gpio);
	
	gpio.GPIO_Pin = GPIO_Pin_4;
  gpio.GPIO_Mode = GPIO_Mode_AF_PP;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);
  
  GPIOA->BSRR |= GPIO_Pin_4;  // GPIOA Pin 4 (NSS) is HIGH (for hardware SPI support)
	
	gpio.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;  // bitmask 0x07
  gpio.GPIO_Mode = GPIO_Mode_Out_PP;
  gpio.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &gpio);
  
  SPI_I2S_DeInit(SPI1); 
  
  SPI_StructInit(&spi); 
  
	// Configure SPI
	spi.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	spi.SPI_Mode = SPI_Mode_Master;
	spi.SPI_DataSize = SPI_DataSize_8b;
	spi.SPI_CPOL = SPI_CPOL_High;
	spi.SPI_CPHA = SPI_CPHA_2Edge;
	spi.SPI_NSS = SPI_NSS_Soft;
	spi.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
	spi.SPI_FirstBit = SPI_FirstBit_MSB;
	//spi.SPI_CRCPolynomial = 7;
	
	SPI_Init(SPI1, &spi);
	SPI_Cmd(SPI1, ENABLE);           /* Enable the SPI  */  
	
}

uint8_t spi0_transfer(uint8_t data){
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

	SPI1->DR = data;
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

	return SPI1->DR; 
}
