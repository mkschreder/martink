obj-y += arch/time.o
obj-y += arch/uart.o
obj-y += arch/gpio.o
obj-y += arch/spi.o 
obj-y += arch/twi.o
obj-y += arch/soc.o
obj-y += arch/pwm.o
obj-y += arch/i2cblk.o

docs-y += arch/README.md

ifeq ($(CONFIG_AVR), y)
	include $(srctree)/arch/avr/Makefile
endif

ifeq ($(CONFIG_NATIVE), y)
	include $(srctree)/arch/native/Makefile
endif

ifeq ($(CONFIG_ARM), y)
	include $(srctree)/arch/arm/Makefile
endif

ifeq ($(CONFIG_AT91SAM3), y)
	include $(srctree)/arch/arm/sam3/Makefile
endif
