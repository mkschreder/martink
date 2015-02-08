COMMON_FLAGS += -I$(srctree)/arch/arm/stm32/CMSIS/ --specs=nano.specs -Wl,-u,vsnprintf
LDFLAGS += -nostartfiles -nostdlib 

ifeq ($(CONFIG_STM32F103), y)
	include $(srctree)/arch/arm/stm32/f10x/Makefile
endif

obj-y += arch/arm/stm32/uart.o
obj-y += arch/arm/stm32/twi.o
obj-y += arch/arm/stm32/spi.o
obj-y += arch/arm/stm32/time.o
obj-y += arch/arm/stm32/gpio.o
obj-y += arch/arm/stm32/pwm.o


obj-y += arch/arm/stm32/usb/usb_core.o
obj-y += arch/arm/stm32/usb/usb_init.o
obj-y += arch/arm/stm32/usb/usb_int.o
obj-y += arch/arm/stm32/usb/usb_mem.o
obj-y += arch/arm/stm32/usb/usb_regs.o
obj-y += arch/arm/stm32/usb/usb_sil.o
