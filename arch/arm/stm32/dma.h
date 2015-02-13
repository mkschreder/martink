 #pragma once 
 
// dma functions
typedef enum {
	// dma channel 1
	DMA11_ADC1,
	DMA11_TIM2_CH3, 
	DMA11_TIM4_CH1, 
	// dma channel 2
	DMA12_SPI1_RX, 
	DMA12_USART3_TX,
	DMA12_TIM1_CH1,
	DMA12_TIM2_UP, 
	DMA12_TIM3_CH3, 
	// dma channel 3
	DMA13_SPI1_TX, 
	DMA13_USART3_RX, 
	DMA13_TIM1_CH2,
	DMA13_TIM3_CH4, 
	DMA13_TIM3_UP, 
	// dma channel 4
	DMA14_SPI2_RX, 
	DMA14_USART1_TX, 
	DMA14_I2C2_TX, 
	DMA14_TIM1_CH4,
	DMA14_TIM1_TRIG, 
	DMA14_TIM1_COM, 
	DMA14_TIM4_CH2, 
	// dma channel 5
	DMA15_SPI2_TX, 
	DMA15_USART1_RX, 
	DMA15_I2C2_RX, 
	DMA15_TIM1_UP, 
	DMA15_TIM2_CH1,
	DMA15_TIM4_CH3, 
	// dma channel 6
	DMA16_USART2_RX, 
	DMA16_I2C1_TX,
	DMA16_TIM1_CH3,
	DMA16_TIM3_CH1,
	DMA16_TIM3_TRIG, 
	// dma channel 7
	DMA17_USART2_TX,
	DMA17_I2C1_RX, 
	DMA17_TIM2_CH2,
	DMA17_TIM2_CH4,
	DMA17_TIM4_UP, 
	
	DMA_CHAN_COUNT, 
} dma_channel_t; 

int8_t dma_configure(dma_channel_t id, uint8_t *to, uint8_t *from, uint16_t size); 
int8_t dma_enable(dma_channel_t id); 
int8_t dma_wait(dma_channel_t id); 
