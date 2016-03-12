VPATH += arch/arm

CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AS := arm-none-eabi-as
LD := arm-none-eabi-gcc
LDXX := arm-none-eabi-g++

ifeq ($(CONFIG_CPU_STM32), y)
	include $(srctree)/arch/arm/stm32/Makefile
endif
ifeq ($(CONFIG_CPU_AT91SAM3), y)
	include $(srctree)/arch/arm/sam3/Makefile
endif

obj-y += arch/arm/arm.o 
obj-y += arch/arm/memory.o
obj-y += arch/arm/syscalls.o
obj-y += arch/arm/atomic.o

docs-y += arch/arm/README.md
