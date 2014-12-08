VPATH += arch/stm32/f10x

CC := arm-linux-gnueabi-gcc
AS := arm-linux-gnueabi-as
CXX := arm-linux-gnueabi-g++
LD := arm-linux-gnueabi-gcc
LDXX := arm-linux-gnueabi-g++
#CPU_FREQ := 16000000
#ARCH_CPU := atmega328p
COMMON_FLAGS += -DSTM32F10X_MD -DminiSTM32F103C8 -nostartfiles -Wl,--gc-sections,-T$(srctree)/arch/stm32/f10x/stm32flash.ld -nostdlib -mcpu=cortex-m3 -mthumb

LDFLAGS += -lgcc
CPU_FLAGS := 

#-DF_CPU=$(CPU_FREQ) -mmcu=$(ARCH_CPU)

obj-$(CONFIG_STM32F10X) += init.o stm32f10x_cec.o stm32f10x_exti.o stm32f10x_iwdg.o stm32f10x_spi.o \
misc.o           stm32f10x_crc.o     stm32f10x_flash.o  stm32f10x_pwr.o   stm32f10x_tim.o \
stm32f10x_adc.o  stm32f10x_dac.o     stm32f10x_fsmc.o   stm32f10x_rcc.o   stm32f10x_usart.o \
stm32f10x_bkp.o  stm32f10x_dbgmcu.o  stm32f10x_gpio.o   stm32f10x_rtc.o   stm32f10x_wwdg.o \
stm32f10x_can.o  stm32f10x_dma.o     stm32f10x_i2c.o    stm32f10x_sdio.o  system_stm32f10x.o \
