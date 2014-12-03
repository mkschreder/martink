VPATH += arch/m328p

CC := avr-gcc
CXX := avr-g++
LD := avr-gcc
LDXX := avr-g++
CPU_FREQ := 16000000
ARCH_CPU := atmega328p

CPU_FLAGS := -DF_CPU=$(CPU_FREQ) -mmcu=$(ARCH_CPU)

obj-$(CONFIG_ATMEGA328P) += adc.o i2cmaster.o i2csoft.o i2csw.o i2csw_slave.o random.o spi.o stack.o time.o twi_slave.o uart.o
