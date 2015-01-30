COMMON_FLAGS += -Iarch/arm/stm32/f10x/CMSIS/

ifeq ($(CONFIG_STM32F103), y)
	include $(srctree)/arch/arm/stm32/f10x/Makefile
endif

obj-y += arch/arm/stm32/uart.o
