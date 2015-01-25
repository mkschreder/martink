VPATH += arch/mega

CC := avr-gcc
CXX := avr-g++
LD := avr-gcc
LDXX := avr-g++

mcu-$(CONFIG_ATMEGA103) := atmega103
mcu-$(CONFIG_ATMEGA88) := atmega88
mcu-$(CONFIG_ATMEGA128) := atmega128
mcu-$(CONFIG_ATMEGA168) := atmega168
mcu-$(CONFIG_ATMEGA2560) := atmega2560
mcu-$(CONFIG_ATMEGA328P) := atmega328p
mcu-$(CONFIG_ATMEGA48) := atmega48
mcu-$(CONFIG_ATMEGA644) := atmega644

cpuflags-y := -mmcu=$(mcu-y)

ARCH := avr-$(mcu-y)

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
