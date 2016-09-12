VPATH += arch/arm

ARCH:=stm32
BOARD:=generic

ifeq ($(CONFIG_CPU_STM32F10x), y)
CPU:=stm32f10x
LDFLAGS+=-llibk-arm32 -Wl,-Tstm32f10x_md.ld 
else ifeq ($(CONFIG_CPU_STM32F30x), y)
CPU:=stm32f30x
else ifeq ($(CONFIG_CPU_STM32F37x), y)
CPU:=stm32f37x
else ifeq ($(CONFIG_CPU_STM32L1xx), y)
CPU:=stm32l1xx
else
$(warning selected cpu is not supported)
endif


CC := arm-none-eabi-gcc
CXX := arm-none-eabi-g++
AS := arm-none-eabi-as
LD := arm-none-eabi-gcc
LDXX := arm-none-eabi-g++

ifeq ($(CONFIG_CPU_AT91SAM3), y)
include $(srctree)/arch/arm/sam3/Makefile
endif

obj-y += arch/arm/arm.o 
obj-y += arch/arm/memory.o
obj-y += arch/arm/syscalls.o
obj-y += arch/arm/atomic.o

docs-y += arch/arm/README.md
