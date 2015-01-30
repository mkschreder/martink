
obj-y += arch/arm/uart.o arch/arm/twi.o arch/arm/spi.o
obj-$(CONFIG_AT91SAM3) += arch/arm/at91sam3xe.o


ifeq ($(CONFIG_STM32F103), y)
	include $(srctree)/arch/arm/stm32/Makefile
endif
ifeq ($(CONFIG_AT91SAM3), y)
	include $(srctree)/arch/arm/sam3/Makefile
endif
