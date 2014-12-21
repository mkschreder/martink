obj-y += \
arch/time.o arch/soc.o arch/uart.o arch/spi.o \
arch/twi.o arch/gpio.o

ifeq ($(CONFIG_AVR), y)
	include $(srctree)/arch/avr/m328p/Makefile
endif

ifeq ($(CONFIG_NATIVE), y)
	include $(srctree)/arch/native/Makefile
endif

ifeq ($(CONFIG_ARM), y)
	include $(srctree)/arch/arm/Makefile
endif

ifeq ($(CONFIG_STM32), y)
	include $(srctree)/arch/arm/stm32/Makefile
endif

ifeq ($(CONFIG_AT91SAM3), y)
	include $(srctree)/arch/arm/sam3/Makefile
endif
