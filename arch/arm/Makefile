VPATH += arch/arm

CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
LD := arm-none-eabi-gcc
LDXX := arm-none-eabi-g++

ifeq ($(CONFIG_STM32F103), y)
	ARCH := arm-stm32f103
endif

obj-y += arch/arm/arm.o
