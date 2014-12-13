VPATH += arch/m328p

CC := avr-gcc
CXX := avr-g++
LD := avr-gcc
LDXX := avr-g++
CPU_FREQ := 16000000
ARCH_CPU := atmega328p

CPU_FLAGS := -DF_CPU=$(CPU_FREQ) -mmcu=$(ARCH_CPU)

obj-$(CONFIG_ATMEGA328P) += \
arch/avr/m328p/adc.o 		arch/avr/m328p/i2cmaster.o 		  \
arch/avr/m328p/random.o  arch/avr/m328p/gpio.o \
arch/avr/m328p/stack.o 				arch/avr/m328p/time.o  \
arch/avr/m328p/twi_slave.o 
