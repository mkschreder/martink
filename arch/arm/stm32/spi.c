#include <arch/soc.h>
#include "spi.h"

#include <kernel/device.h>

#define SPI_TIMEOUT 10

struct spi_device {
	SPI_TypeDef *spi; 
	GPIO_TypeDef *gpio; 
	int rcc_gpio; 
	int rcc_spi; 
	int apb_id; 
	int out_pins, in_pins, nss_pin; 
}; 

static const struct spi_device _spi[] = {
	{
		.spi = SPI1, 
		.gpio = GPIOA, 
		.rcc_gpio = RCC_APB2Periph_GPIOA, 
		.rcc_spi = RCC_APB2Periph_SPI1, 
		.apb_id = 2, 
		.out_pins = GPIO_Pin_5 | GPIO_Pin_7 | GPIO_Pin_6, 
		.in_pins = GPIO_Pin_6,
		.nss_pin = GPIO_Pin_4
	}, 
	{
		.spi = SPI2, 
		.gpio = GPIOB, 
		.rcc_gpio = RCC_APB2Periph_GPIOB, 
		.rcc_spi = RCC_APB1Periph_SPI2, 
		.apb_id = 1, 
		.out_pins = GPIO_Pin_13 | GPIO_Pin_15 | GPIO_Pin_12, 
		.in_pins = GPIO_Pin_14,
		.nss_pin = GPIO_Pin_12
	}
}; 

int16_t spi_init_device(uint8_t dev){
	GPIO_InitTypeDef gpio;
	SPI_InitTypeDef spi; 
	
	const uint8_t count = sizeof(_spi) / sizeof(_spi[0]); 
	
	if(dev > count) return -1; 
	
	const struct spi_device *d = &_spi[dev]; 
	
	RCC_APB2PeriphClockCmd ( d->rcc_gpio | RCC_APB2Periph_AFIO, ENABLE); 
	
	if(d->apb_id == 1)
		RCC_APB1PeriphClockCmd ( d->rcc_spi, ENABLE); 
	else if(d->apb_id == 2)
		RCC_APB2PeriphClockCmd ( d->rcc_spi, ENABLE); 
	else
		return -1; 
		
	// Configure SCK and MOSI 
	gpio.GPIO_Pin = d->out_pins;
	gpio.GPIO_Mode = GPIO_Mode_AF_PP;
	gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(d->gpio, &gpio);

	// Configure MISO
	gpio.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	gpio.GPIO_Pin = d->in_pins;
	GPIO_Init(d->gpio, &gpio);
  
  d->gpio->BSRR |= d->nss_pin;  // GPIOA Pin 4 (NSS) is HIGH (for hardware SPI support)
	
  SPI_I2S_DeInit(d->spi); 
  
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
	
	SPI_Init(d->spi, &spi);
	SPI_Cmd(d->spi, ENABLE);           /* Enable the SPI  */ 
	
	return 0;  
}

uint8_t spi_transfer(uint8_t dev, uint8_t data){
	const uint8_t count = sizeof(_spi) / sizeof(_spi[0]); 
	if(dev > count) return 0xff; 
	const struct spi_device *d = &_spi[dev]; 
	
	wait_on_us(SPI_I2S_GetFlagStatus(d->spi, SPI_I2S_FLAG_TXE) == RESET, SPI_TIMEOUT);

	d->spi->DR = data;
	wait_on_us(SPI_I2S_GetFlagStatus(d->spi, SPI_I2S_FLAG_RXNE) == RESET, SPI_TIMEOUT);

	return d->spi->DR; 
}

