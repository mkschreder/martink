#include "dma.h"

/*
// dma channel 1
	DMA_ADC1,
	DMA_TIM2_CH3, 
	DMA_TIM4_CH1, 
	// dma channel 2
	DMA_SPI1_RX, 
	DMA_USART3_TX,
	DMA_TIM1_CH1,
	DMA_TIM2_UP, 
	DMA_TIM3_CH3, 
	// dma channel 3
	DMA_SPI1_TX, 
	DMA_USART3_RX, 
	DMA_TIM1_CH2,
	DMA_TIM3_CH4, 
	DMA_TIM3_UP, 
	// dma channel 4
	DMA_SPI2_RX, 
	DMA_USART1_TX, 
	DMA_I2C2_TX, 
	DMA_TIM1_CH4,
	DMA_TIM1_TRIG, 
	DMA_TIM1_COM, 
	DMA_TIM4_CH2, 
	// dma channel 5
	DMA_SPI2_TX, 
	DMA_USART1_RX, 
	DMA_I2C2_RX, 
	DMA_TIM1_UP, 
	DMA_TIM2_CH1,
	DMA_TIM4_CH3, 
	// dma channel 6
	DMA_USART2_RX, 
	DMA_I2C1_TX,
	DMA_TIM1_CH3,
	DMA_TIM3_CH1,
	DMA_TIM3_TRIG, 
	// dma channel 7
	DMA_USART2_TX,
	DMA_I2C1_RX, 
	DMA_TIM2_CH2,
	DMA_TIM2_CH4,
	DMA_TIM4_UP, 
	
	// special 
	DMA_MEM2MEM, 
*/

static struct dma_conf {
	int channel_id;
	DMA_InitTypeDef *config; 
} _dma_channels[DMA_CHAN_COUNT] = { 
	// dma channel 1 
	//DMA_ADC1 
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	//DMA_TIM2_CH3
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	//DMA_TIM4_CH1
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	// dma channel 2
	//DMA_SPI1_RX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	//DMA_USART3_TX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	//DMA_TIM1_CH1
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	//DMA_TIM2_UP
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	}, 
	//DMA_TIM3_CH3
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	// dma channel 3
	//DMA_SPI1_TX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_USART3_RX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM1_CH2
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM3_CH4
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM3_UP
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	// dma channel 4
	//DMA_SPI2_RX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_USART1_TX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_I2C2_TX
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM1_CH4
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM1_TRIG
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM1_COM
	{
		.channel_id = DMA_Channel1, 
		.config = {}
	},
	//DMA_TIM4_CH2
}; 

void dma_memcpy(void *dst, void *src, size_t size){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	DMA_InitTypeDef  DMA_InitStructure;
	
	DMA_DeInit(DMA1_Channel1); 
	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Enable; // memory to memory 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal; //medium priority
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //source and destination data size word=32bit
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;//automatic memory increment enable. Destination and source
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Enable; // source location incremented at each byte
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//chunk of data to be transfered
	DMA_InitStructure.DMA_BufferSize = size;//source and destination start addresses
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)src;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dst;//send values to DMA registers
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	// Enable DMA1 Channel Transfer Complete interrupt
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	 
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	 
	//Enable DMA1 Channel transfer
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void){
	//Test on DMA1 Channel1 Transfer Complete interrupt
	if(DMA_GetITStatus(DMA1_IT_TC1)){
		status=1;
		//Clear DMA1 Channel1 Half Transfer, Transfer Complete and Global interrupt pending bits
		DMA_ClearITPendingBit(DMA1_IT_GL1);
	}
}
