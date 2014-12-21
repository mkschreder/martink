VPATH += arch/mega

CC := avr-gcc
CXX := avr-g++
LD := avr-gcc
LDXX := avr-g++

cpuflags-$(CONFIG_ATMEGA103) := -mmcu=atmega103
cpuflags-$(CONFIG_ATMEGA88) := -mmcu=atmega88
cpuflags-$(CONFIG_ATMEGA128) := -mmcu=atmega128
cpuflags-$(CONFIG_ATMEGA168) := -mmcu=atmega168
cpuflags-$(CONFIG_ATMEGA2560) := -mmcu=atmega2560
cpuflags-$(CONFIG_ATMEGA328P) := -mmcu=atmega328p
cpuflags-$(CONFIG_ATMEGA48) := -mmcu=atmega48
cpuflags-$(CONFIG_ATMEGA644) := -mmcu=atmega644

#ifndef CPU_FREQ
#$(error CPU_FREQ must be set for avr arch!)
#endif

CPU_FREQ := 16000000
#ARCH_CPU := atmega328p

CPU_FLAGS := -DF_CPU=$(CPU_FREQ) $(cpuflags-y)

obj-$(CONFIG_ATMEGA328P) += \
arch/avr/mega/adc.o 				arch/avr/mega/twi.o 		  \
arch/avr/mega/gpio.o \
arch/avr/mega/stack.o 			arch/avr/mega/time.o  \
arch/avr/mega/twi_slave.o  arch/avr/mega/uart.o
