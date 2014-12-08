obj-y += arch/time.o 

ifeq ($(CONFIG_ATMEGA328P), y)
	include $(srctree)/arch/avr/m328p/Makefile
endif

ifeq ($(CONFIG_NATIVE), y)
	include $(srctree)/arch/native/Makefile
endif

ifeq ($(CONFIG_STM32), y)
	include $(srctree)/arch/arm/stm32/Makefile
endif

ifeq ($(CONFIG_SAM3), y)
	include $(srctree)/arch/arm/sam3/Makefile
endif
