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

/* Linker script for STM32F103 Cortex-M3 ARM MCU */

/* $Id: linker.ld 1613 2009-09-16 08:27:32Z svn $ */

MEMORY
{
  flash (rx ) : ORIGIN = 0x20000200, LENGTH = 0x2000
  ram   (rwx) : ORIGIN = 0x20002200, LENGTH = 0x4000
}

__rom_start__	= 0x20000200;
__rom_size__	= 0x00002200;
__ram_start__	= 0x20002200;
__ram_size__	= 0x00004000;

__stack_top__	= __ram_start__ + __ram_size__;

PROVIDE(STACK_TOP = __stack_top__);

SECTIONS
{
  . = 0x00000000;
  .text : {
  *(.text)				/* Program code */
  *(.rodata)				/* Read only data */
  *(.rodata*)
  *(.glue_7)
  *(.glue_7t)
  *(.eh_frame)
  . = ALIGN(4);
  __text_end__ = .;			/* Used in crt0.S */
  } >flash

/* C++ exception unwinding stuff--needed by Code Sourcery toolchain */

.ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >flash

__exidx_start = .;
.ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } >flash
__exidx_end = .;

  . = 0x20000000;
  .data : {
  . = ALIGN(4);
  __data_beg__ = .;			/* Used in crt0.S */
  *(.data)				/* Initialized datay */
  . = ALIGN(4);
  __data_end__ = .;			/* Used in crt0.S */
  } >ram AT > flash 

  .bss : {
  . = ALIGN(4);
  __bss_beg__ = .;			/* Used in crt0.S */
  *(.bss)				/* Uninitialized data */
  *(COMMON)				/* Common data */
  . = ALIGN(4);
  __bss_end__ = .;			/* Used in crt0.S */
  } >ram

  . = ALIGN(4);
  end = .;
}  

/*  Libraries to link against. */

/*  Provide a default vector for any unhandled interrupts. */

PROVIDE(NMIException = _default_handler);
PROVIDE(HardFaultException = _default_handler);
PROVIDE(MemManageException = _default_handler);
PROVIDE(BusFaultException = _default_handler);
PROVIDE(UsageFaultException = _default_handler);
PROVIDE(DebugMonitor = _default_handler);
PROVIDE(SVCHandler = _default_handler);
PROVIDE(PendSVC = _default_handler);
PROVIDE(SysTickHandler = _default_handler);
PROVIDE(WWDG_IRQHandler = _default_handler);
PROVIDE(PVD_IRQHandler = _default_handler);
PROVIDE(TAMPER_IRQHandler = _default_handler);
PROVIDE(RTC_IRQHandler = _default_handler);
PROVIDE(FLASH_IRQHandler = _default_handler);
PROVIDE(RCC_IRQHandler = _default_handler);
PROVIDE(EXTI0_IRQHandler = _default_handler);
PROVIDE(EXTI1_IRQHandler = _default_handler);
PROVIDE(EXTI2_IRQHandler = _default_handler);
PROVIDE(EXTI3_IRQHandler = _default_handler);
PROVIDE(EXTI4_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel1_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel2_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel3_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel4_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel5_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel6_IRQHandler = _default_handler);
PROVIDE(DMA1_Channel7_IRQHandler = _default_handler);
PROVIDE(ADC1_2_IRQHandler = _default_handler);
PROVIDE(USB_HP_CAN_TX_IRQHandler = _default_handler);
PROVIDE(USB_LP_CAN_RX0_IRQHandler = _default_handler);
PROVIDE(CAN_RX1_IRQHandler = _default_handler);
PROVIDE(CAN_SCE_IRQHandler = _default_handler);
PROVIDE(EXTI9_5_IRQHandler = _default_handler);
PROVIDE(TIM1_BRK_IRQHandler = _default_handler);
PROVIDE(TIM1_UP_IRQHandler = _default_handler);
PROVIDE(TIM1_TRG_COM_IRQHandler = _default_handler);
PROVIDE(TIM1_CC_IRQHandler = _default_handler);
PROVIDE(TIM2_IRQHandler = _default_handler);
PROVIDE(TIM3_IRQHandler = _default_handler);
PROVIDE(TIM4_IRQHandler = _default_handler);
PROVIDE(I2C1_EV_IRQHandler = _default_handler);
PROVIDE(I2C1_ER_IRQHandler = _default_handler);
PROVIDE(I2C2_EV_IRQHandler = _default_handler);
PROVIDE(I2C2_ER_IRQHandler = _default_handler);
PROVIDE(SPI1_IRQHandler = _default_handler);
PROVIDE(SPI2_IRQHandler = _default_handler);
PROVIDE(USART1_IRQHandler = _default_handler);
PROVIDE(USART2_IRQHandler = _default_handler);
PROVIDE(USART3_IRQHandler = _default_handler);
PROVIDE(EXTI15_10_IRQHandler = _default_handler);
PROVIDE(RTCAlarm_IRQHandler = _default_handler);
PROVIDE(USBWakeUp_IRQHandler = _default_handler);
PROVIDE(TIM8_BRK_IRQHandler = _default_handler);
PROVIDE(TIM8_UP_IRQHandler = _default_handler);
PROVIDE(TIM8_TRG_COM_IRQHandler = _default_handler);
PROVIDE(TIM8_CC_IRQHandler = _default_handler);
PROVIDE(ADC3_IRQHandler = _default_handler);
PROVIDE(FSMC_IRQHandler = _default_handler);
PROVIDE(SDIO_IRQHandler = _default_handler);
PROVIDE(TIM5_IRQHandler = _default_handler);
PROVIDE(SPI3_IRQHandler = _default_handler);
PROVIDE(UART4_IRQHandler = _default_handler);
PROVIDE(UART5_IRQHandler = _default_handler);
PROVIDE(TIM6_IRQHandler = _default_handler);
PROVIDE(TIM7_IRQHandler = _default_handler);
PROVIDE(DMA2_Channel1_IRQHandler = _default_handler);
PROVIDE(DMA2_Channel2_IRQHandler = _default_handler);
PROVIDE(DMA2_Channel3_IRQHandler = _default_handler);
PROVIDE(DMA2_Channel4_5_IRQHandler = _default_handler);
