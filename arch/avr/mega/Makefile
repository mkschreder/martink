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


obj-$(CONFIG_CPU_ATMEGA328P) += \
	arch/avr/mega/time.o 

define target/image/finalize
	avr-objcopy -j .text -j .data -O ihex $(1) $(2).hex
	avr-size --format=berkeley $(1)
endef

flash-arduino: all
	sudo avrdude -P /dev/ttyUSB0 -b 57600 -c arduino -p m328p -U flash:w:$(TARGET).hex

PHONY += flash-arduino
