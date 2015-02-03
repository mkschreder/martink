/*
  stm32flash - Open Source ST STM32 flash program for *nix
  Copyright (C) 2010 Geoffrey McRae <geoff@spacevs.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* Startup for STM32F103 Cortex-M3 ARM MCU */

// $Id: crt0.S 1613 2009-09-16 08:27:32Z svn $

		.text
		.extern		main
		.global		 _start
		.global		_default_handler
		.code		16
		.syntax 	unified
		.type		_reset, function

		.extern		__bss_beg__
		.extern		__bss_end__
		.extern		__data_beg__
		.extern		__data_end__
		.extern		__text_end__
		.extern		STACK_TOP

// Exception vector table

_start: 	.word		STACK_TOP
		.word		_reset
		.extern		NMIException
		.word		NMIException
		.extern		HardFaultException
		.word		HardFaultException
		.extern		MemManageException
		.word		MemManageException
		.extern		BusFaultException
		.word		BusFaultException
		.extern		UsageFaultException
		.word		UsageFaultException
		.word		0, 0, 0, 0
		.extern		SVCHandler
		.word		SVCHandler
		.extern		DebugMonitor
		.word		DebugMonitor
		.word		0
		.extern		PendSVC
		.word		PendSVC
		.extern		SysTickHandler
		.word		SysTickHandler
		.extern		WWDG_IRQHandler
		.word		WWDG_IRQHandler
		.extern		PVD_IRQHandler
		.word		PVD_IRQHandler
		.extern		TAMPER_IRQHandler
		.word		TAMPER_IRQHandler
		.extern		RTC_IRQHandler
		.word		RTC_IRQHandler
		.extern		FLASH_IRQHandler
		.word		FLASH_IRQHandler
		.extern		RCC_IRQHandler
		.word		RCC_IRQHandler
		.extern		EXTI0_IRQHandler
		.word		EXTI0_IRQHandler
		.extern		EXTI1_IRQHandler
		.word		EXTI1_IRQHandler
		.extern		EXTI2_IRQHandler
		.word		EXTI2_IRQHandler
		.extern		EXTI3_IRQHandler
		.word		EXTI3_IRQHandler
		.extern		EXTI4_IRQHandler
		.word		EXTI4_IRQHandler
		.extern		DMA1_Channel1_IRQHandler
		.word		DMA1_Channel1_IRQHandler
		.extern		DMA1_Channel2_IRQHandler
		.word		DMA1_Channel2_IRQHandler
		.extern		DMA1_Channel3_IRQHandler
		.word		DMA1_Channel3_IRQHandler
		.extern		DMA1_Channel4_IRQHandler
		.word		DMA1_Channel4_IRQHandler
		.extern		DMA1_Channel5_IRQHandler
		.word		DMA1_Channel5_IRQHandler
		.extern		DMA1_Channel6_IRQHandler
		.word		DMA1_Channel6_IRQHandler
		.extern		DMA1_Channel7_IRQHandler
		.word		DMA1_Channel7_IRQHandler
		.extern		ADC1_2_IRQHandler
		.word		ADC1_2_IRQHandler
		.extern		USB_HP_CAN_TX_IRQHandler
		.word		USB_HP_CAN_TX_IRQHandler
		.extern		USB_LP_CAN_RX0_IRQHandler
		.word		USB_LP_CAN_RX0_IRQHandler
		.extern		CAN_RX1_IRQHandler
		.word		CAN_RX1_IRQHandler
		.extern		CAN_SCE_IRQHandler
		.word		CAN_SCE_IRQHandler
		.extern		EXTI9_5_IRQHandler
		.word		EXTI9_5_IRQHandler
		.extern		TIM1_BRK_IRQHandler
		.word		TIM1_BRK_IRQHandler
		.extern		TIM1_UP_IRQHandler
		.word		TIM1_UP_IRQHandler
		.extern		TIM1_TRG_COM_IRQHandler
		.word		TIM1_TRG_COM_IRQHandler
		.extern		TIM1_CC_IRQHandler
		.word		TIM1_CC_IRQHandler
		.extern		TIM2_IRQHandler
		.word		TIM2_IRQHandler
		.extern		TIM3_IRQHandler
		.word		TIM3_IRQHandler
		.extern		TIM4_IRQHandler
		.word		TIM4_IRQHandler
		.extern		I2C1_EV_IRQHandler
		.word		I2C1_EV_IRQHandler
		.extern		I2C1_ER_IRQHandler
		.word		I2C1_ER_IRQHandler
		.extern		I2C2_EV_IRQHandler
		.word		I2C2_EV_IRQHandler
		.extern		I2C2_ER_IRQHandler
		.word		I2C2_ER_IRQHandler
		.extern		SPI1_IRQHandler
		.word		SPI1_IRQHandler
		.extern		SPI2_IRQHandler
		.word		SPI2_IRQHandler
		.extern		USART1_IRQHandler
		.word		USART1_IRQHandler
		.extern		USART2_IRQHandler
		.word		USART2_IRQHandler
		.extern		USART3_IRQHandler
		.word		USART3_IRQHandler
		.extern		EXTI15_10_IRQHandler
		.word		EXTI15_10_IRQHandler
		.extern		RTCAlarm_IRQHandler
		.word		RTCAlarm_IRQHandler
		.extern		USBWakeUp_IRQHandler
		.word		USBWakeUp_IRQHandler
		.extern		TIM8_BRK_IRQHandler
		.word		TIM8_BRK_IRQHandler
		.extern		TIM8_UP_IRQHandler
		.word		TIM8_UP_IRQHandler
		.extern		TIM8_TRG_COM_IRQHandler
		.word		TIM8_TRG_COM_IRQHandler
		.extern		TIM8_CC_IRQHandler
		.word		TIM8_CC_IRQHandler
		.extern		ADC3_IRQHandler
		.word		ADC3_IRQHandler
		.extern		FSMC_IRQHandler
		.word		FSMC_IRQHandler
		.extern		SDIO_IRQHandler
		.word		SDIO_IRQHandler
		.extern		TIM5_IRQHandler
		.word		TIM5_IRQHandler
		.extern		SPI3_IRQHandler
		.word		SPI3_IRQHandler
		.extern		UART4_IRQHandler
		.word		UART4_IRQHandler
		.extern		UART5_IRQHandler
		.word		UART5_IRQHandler
		.extern		TIM6_IRQHandler
		.word		TIM6_IRQHandler
		.extern		TIM7_IRQHandler
		.word		TIM7_IRQHandler
		.extern		DMA2_Channel1_IRQHandler
		.word		DMA2_Channel1_IRQHandler
		.extern		DMA2_Channel2_IRQHandler
		.word		DMA2_Channel2_IRQHandler
		.extern		DMA2_Channel3_IRQHandler
		.word		DMA2_Channel3_IRQHandler
		.extern		DMA2_Channel4_5_IRQHandler
		.word		DMA2_Channel4_5_IRQHandler

// Default exception handler; does nothing

_default_handler: bx		lr

// Reset vector: Set up environment to call C main()

_reset:

// Copy initialized data from flash to RAM

copy_data:	ldr		r1, DATA_BEG
		ldr 		r2, TEXT_END
		ldr 		r3, DATA_END
		subs		r3, r3, r1		// Length of initialized data
		beq		zero_bss		// Skip if none

copy_data_loop: ldrb		r4, [r2], #1		// Read byte from flash
		strb		r4, [r1], #1  		// Store byte to RAM
		subs		r3, r3, #1  		// Decrement counter
		bgt 		copy_data_loop		// Repeat until done

// Zero uninitialized data (bss)

zero_bss: 	ldr 		r1, BSS_BEG
		ldr 		r3, BSS_END
		subs 		r3, r3, r1		// Length of uninitialized data
		beq		call_main		// Skip if none

		mov 		r2, #0

zero_bss_loop: 	strb		r2, [r1], #1		// Store zero
		subs		r3, r3, #1		// Decrement counter
		bgt		zero_bss_loop		// Repeat until done

// Call main()

call_main:	mov		r0, #0			// argc=0
		mov		r1, #0			// argv=NULL

		bl		main 

// main() should never return, but if it does just do nothing forever
// Should probably put processor into sleep mode instead.

endless_loop:	b		endless_loop

// These are filled in by the linker
	
TEXT_END:	.word		__text_end__
DATA_BEG:	.word		__data_beg__
DATA_END:	.word		__data_end__
BSS_BEG:	.word		__bss_beg__ 
BSS_END:	.word		__bss_end__

		.end
