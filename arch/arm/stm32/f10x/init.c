
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"

extern int __init_array_start;
extern int __init_array_end;

static void ConstructorsInit(void)
{
	int *s, *e;

	// call the constructorts of global objects
	s = &__init_array_start;
	e = &__init_array_end;
	while (s != e)
	{
		(*((void (**)(void)) s++))();
	}
}

extern int main(void); 
//extern void SystemInit(void); 

void c_startup(void); 
extern char _ramstart, _estack, _etext, _sdata, _sbss, _ebss, _edata; 

/*
static void RCC_Configuration(void){
	SystemCoreClock = HSE_VALUE; 
	
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
	if (HSEStartUpStatus == SUCCESS){
		#if defined (STM32F10X_LD_VL) || (defined STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
		RCC_PREDIV1Config(RCC_PREDIV1_Source_HSE,RCC_PREDIV1_Div1);
		#endif
		
		// set default HCLK
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		
		uint32_t pllmul[] = {1, 1,
				RCC_PLLMul_2, RCC_PLLMul_3, RCC_PLLMul_4, RCC_PLLMul_5, RCC_PLLMul_6, RCC_PLLMul_7, 
				RCC_PLLMul_8, RCC_PLLMul_9, RCC_PLLMul_10, RCC_PLLMul_11, RCC_PLLMul_12, RCC_PLLMul_13, 
				RCC_PLLMul_14, RCC_PLLMul_15, RCC_PLLMul_16}; 
		uint32_t cpu_clock = 72000000; 
		#if defined (STM32F10X_LD_VL) || (defined STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
			cpu_clock = 24000000; 
		#endif
		
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_2);
		SystemCoreClock = HSE_VALUE * 2; 
		
		// find suitable pll setting depending on HSE_VALUE
		//SystemCoreClock = (((HSE_VALUE / prediv2factor) * pll2mull) / prediv1factor) * pllmull; 
		for(unsigned c = 2; c < sizeof(pllmul) / sizeof(pllmul[0]); c++){
			if((HSE_VALUE * c) > cpu_clock){
				RCC_PLLConfig(RCC_PLLSource_HSE_Div1, pllmul[c-1]);
				SystemCoreClock = HSE_VALUE * (c - 1); 
				break; 
			}
		}
		
		// apb at the same freq as hclk
		RCC_PCLK2Config(RCC_HCLK_Div1);
		RCC_PCLK1Config(RCC_HCLK_Div1);
		// adc at system clock divided by a factor
		RCC_ADCCLKConfig(RCC_PCLK2_Div2);
		// enable pll
		RCC_PLLCmd(ENABLE);
		// check that pll is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
		// set pll as clock source
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		// wait for pll settings to take effect
		while (RCC_GetSYSCLKSource() != 0x08) {}
		
	} 
}
*/

void c_startup(void)
{
	char *src, *dst;
	
	dst = &_ramstart; 
	while(dst < &_estack)
		*(dst++) = 0; 
		
	src = &_etext;
	dst = &_sdata;
	while(dst < &_edata) 
		*(dst++) = *(src++);

	src = &_sbss;
	while(src < &_ebss) 
		*(src++) = 0;
	
	SystemInit(); 
	//RCC_Configuration(); 
	/*
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div4); 
	RCC_PCLK2Config(RCC_HCLK_Div4);
	*/
	
	ConstructorsInit(); 
	
	main();
}


