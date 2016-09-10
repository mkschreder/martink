COMMON_FLAGS += -I$(srctree)/arch/arm/stm32/CMSIS/ 
CFLAGS += -Wno-int-to-pointer-cast -Wno-pointer-to-int-cast
#LDFLAGS += -nostartfiles -nostdlib --specs=nano.specs -Wl,-u,vsnprintf
LDFLAGS += -nostartfiles -nostdlib -lc -lgcc -Wl,-u,vsnprintf

ifeq ($(CONFIG_CPU_STM32F10X), y)
CPU:=stm32f10x
include $(srctree)/arch/arm/stm32/f10x/Makefile
endif

BOARD:=default

obj-y += arch/arm/stm32/uart.o
obj-y += arch/arm/stm32/twi.o
obj-y += arch/arm/stm32/spi.o
obj-y += arch/arm/stm32/time.o
obj-y += arch/arm/stm32/gpio.o
obj-y += arch/arm/stm32/pwm.o
obj-y += arch/arm/stm32/adc.o


obj-y += arch/arm/stm32/usb/usb_core.o
obj-y += arch/arm/stm32/usb/usb_init.o
obj-y += arch/arm/stm32/usb/usb_int.o
obj-y += arch/arm/stm32/usb/usb_mem.o
obj-y += arch/arm/stm32/usb/usb_regs.o
obj-y += arch/arm/stm32/usb/usb_sil.o

define target/image/finalize
	arm-none-eabi-objcopy -O ihex --set-start 0x8000000 $(1) $(2).hex
	arm-none-eabi-size $(1)
endef

define target/image/flash
	stty -F /dev/ttyUSB0 raw speed 115200 -crtscts cs8 -parenb -cstopb -ixon
	echo -n 'R' >/dev/ttyUSB0
	stm32flash -w $(1).hex -v -g 0x0 -b 115200 /dev/ttyUSB0 
endef
