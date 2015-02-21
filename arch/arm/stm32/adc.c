#include <arch/soc.h>

#define ADC_BUFFER_SIZE (8 * 4)

volatile uint16_t _adc_values[ADC_BUFFER_SIZE];

void DMA1_Channel1_IRQHandler(void); 
void DMA1_Channel1_IRQHandler(void){
	//Test on DMA1 Channel1 Transfer Complete interrupt
	if(DMA_GetITStatus(DMA1_IT_TC1)){
		//Clear DMA1 interrupt pending bits
		DMA_ClearITPendingBit(DMA1_IT_GL1);
		
	}
}

static void adc_dma_init(void){
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	DMA_InitTypeDef  DMA_InitStructure;
	DMA_DeInit(DMA1_Channel1);
	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ADC_BUFFER_SIZE;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_adc_values;
	
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
	
	DMA_Cmd(DMA1_Channel1, ENABLE); 
	
	NVIC_InitTypeDef NVIC_InitStructure;
	
	NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

// initializes adc. User must configure proper GPIO as GP_INPUT | GP_ANALOG
void adc_init(void){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE); 
	
	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 8;
	ADC_Init(ADC1, &ADC_InitStructure);
	
	//wake up temperature sensor
	//ADC_TempSensorVrefintCmd(ENABLE);
	
	//configure each channel
	ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 7, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, ADC_SampleTime_41Cycles5);
	
	ADC_Cmd(ADC1, ENABLE);
	
	ADC_DMACmd(ADC1, ENABLE);
	
	ADC_ResetCalibration(ADC1);
	
	while(ADC_GetResetCalibrationStatus(ADC1));
	
	ADC_StartCalibration(ADC1);
	
	while(ADC_GetCalibrationStatus(ADC1));
	
	adc_dma_init(); 

	// first conversion
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// fast adc read from the dma buffer
int8_t adc_start_read(uint8_t channel, uint16_t *result)
{
	if(channel > 8) return -1; 
	*result = (uint16_t)((_adc_values[channel]+_adc_values[channel+8]
					+_adc_values[channel+16]+_adc_values[channel+24])/4); 
	return 0; 
}

uint8_t adc_aquire(uint8_t chan){
	(void)chan; 
	// always allow because adc is running in background
	return 1; 
}

void adc_release(uint8_t chan){
	(void)chan; 
}

uint8_t adc_busy(void){
	return 0; 
}
