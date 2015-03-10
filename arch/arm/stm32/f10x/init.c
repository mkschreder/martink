
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_flash.h"

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


static void RCC_Configuration(void){
	SystemCoreClock = HSE_VALUE; 
	
	RCC_DeInit();
	// 1. Clocking the controller from internal HSI RC (8 MHz)
  RCC_HSICmd(ENABLE);
  // wait until the HSI is ready
  while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET);
  RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
  
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
		uint32_t prediv = RCC_PLLSource_HSI_Div2; 
		uint32_t pll = RCC_PLLMul_2; // default cpu clock is 2 * HSE. 
		
		// limit frequency to 24mhz for value line chips
		#if defined (STM32F10X_LD_VL) || (defined STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
			cpu_clock = 24000000; 
			prediv = RCC_PLLSource_PREDIV1;
		#else 
			// set the default here
			prediv = RCC_PLLSource_HSE_Div1; 
		#endif
		
		SystemCoreClock = HSE_VALUE * 2; 
		
		// find suitable pll setting depending on HSE_VALUE
		// Clock calc: SystemCoreClock = (((HSE_VALUE / prediv2factor) * pll2mull) / prediv1factor) * pllmull; 
		for(unsigned c = 2; c < sizeof(pllmul) / sizeof(pllmul[0]); c++){
			if((HSE_VALUE * c) > cpu_clock){
				pll = pllmul[c-1]; 
				SystemCoreClock = HSE_VALUE * (c - 1); 
				break; 
			}
		}
		
		// configure cpu pll
		RCC_PLLConfig(prediv, pll);
		// enable pll
		RCC_PLLCmd(ENABLE);
		RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
		RCC_ADCCLKConfig(RCC_PCLK2_Div8);
		
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		// ahb at the same freq as hclk
		RCC_PCLK2Config(RCC_HCLK_Div1);
		// low speed apb speed
		RCC_PCLK1Config(RCC_HCLK_Div2);
		// adc at system clock divided by a factor
		RCC_ADCCLKConfig(RCC_PCLK2_Div8);
		
		// 5. Init Embedded Flash
		// Zero wait state, if 0 < HCLK 24 MHz
		// One wait state, if 24 MHz < HCLK 56 MHz
		// Two wait states, if 56 MHz < HCLK 72 MHz
		// Flash wait state
		FLASH_SetLatency(FLASH_Latency_2);
		// Half cycle access
		FLASH_HalfCycleAccessCmd(FLASH_HalfCycleAccess_Disable);
		// Prefetch buffer
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
		
		// 5. Clock system from PLL
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		// check that pll is ready
		while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET) {}
		
		// set pll as clock source
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
		// wait for pll settings to take effect
		while (RCC_GetSYSCLKSource() != 0x08) {}
		
	} 
}

static uint32_t _boot_magic = 0; 

static void _enter_bootloader(void){
	__asm__("LDR	R0, =0x1FFFF000"); 
	__asm__("LDR	SP,[R0, #0]"); 
	__asm__("LDR	R0,[R0, #4]"); 
	__asm__("BX		R0"); 
}

void EnterFlashUpgrade(void); 
void NVIC_GenerateSystemReset(void); 
void EnterFlashUpgrade(void){
	_boot_magic = 0xDEADBEEF; // 64KBSTM32F103
	NVIC_SystemReset();
}

void c_startup(void)
{
	char *src, *dst;
	
	//SystemInit(); 
	RCC_Configuration(); 
	/*
	RCC_HCLKConfig(RCC_SYSCLK_Div1);
	RCC_PCLK1Config(RCC_HCLK_Div4); 
	RCC_PCLK2Config(RCC_HCLK_Div4);
	*/
	
	if(_boot_magic == 0xdeadbeef){
		_enter_bootloader(); 
	}
	
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
	
	ConstructorsInit(); 
	
	main();
}


