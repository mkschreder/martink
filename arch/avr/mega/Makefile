VPATH += arch/avr/mega

ARCH:=avr
CPU:=atmega328p
BOARD:=generic

CC := avr-gcc
CXX := avr-g++
LD := avr-gcc
LDXX := avr-g++

mcu-y:=atmega328p
mcu-$(CONFIG_CPU_ATMEGA103) := atmega103
mcu-$(CONFIG_CPU_ATMEGA88) := atmega88
mcu-$(CONFIG_CPU_ATMEGA128) := atmega128
mcu-$(CONFIG_CPU_ATMEGA168) := atmega168
mcu-$(CONFIG_CPU_ATMEGA2560) := atmega2560
mcu-$(CONFIG_CPU_ATMEGA328P) := atmega328p
mcu-$(CONFIG_CPU_ATMEGA48) := atmega48
mcu-$(CONFIG_CPU_ATMEGA644) := atmega644

CPU_FREQ := 16000000
CPU:=$(mcu-y)

CFLAGS += -mmcu=$(mcu-y) -DF_CPU=$(CPU_FREQ) 
LDFLAGS += -mmcu=$(mcu-y) -lc 


#obj-$(CONFIG_CPU_ATMEGA328P) += \
#	arch/avr/mega/adc.o \
#	arch/avr/mega/twi.o 		  \
	arch/avr/mega/gpio.o \
	arch/avr/mega/time.o  \
	arch/avr/mega/uart.o \
	arch/avr/mega/spi.o

define target/image/finalize
	avr-objcopy -j .text -j .data -O ihex $(1) $(2).hex
	avr-size --format=berkley $(1)
endef
