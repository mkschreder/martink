VPATH += arch/stm32/f10x

COMMON_FLAGS += -Wl,--gc-sections -fno-common -mcpu=cortex-m3 -mthumb 

ifeq ($(CONFIG_STM32F103), y)
	COMMON_FLAGS += -DHSE_VALUE=8000000 -DF_CPU=72000000 -DSTM32F10X_MD -Wl,-L$(srctree)/arch/arm/stm32/f10x/,-Tstm32f10x_md.ld 
endif
ifeq ($(CONFIG_STM32F100MDVL), y)
	COMMON_FLAGS += -DHSE_VALUE=12000000 -DSTM32F10X_MD_VL -Wl,-L$(srctree)/arch/arm/stm32/f10x/,-Tstm32f10x_md_vl.ld 
endif

#LDFLAGS += -lgcc
CPU_FLAGS := 

#-DF_CPU=$(CPU_FREQ) -mmcu=$(ARCH_CPU)

obj-$(CONFIG_STM32) += arch/arm/stm32/f10x/init.o \
arch/arm/stm32/f10x/stm32f10x_cec.o \
arch/arm/stm32/f10x/stm32f10x_exti.o \
arch/arm/stm32/f10x/stm32f10x_iwdg.o \
arch/arm/stm32/f10x/stm32f10x_spi.o \
arch/arm/stm32/f10x/misc.o \
arch/arm/stm32/f10x/stm32f10x_crc.o \
arch/arm/stm32/f10x/stm32f10x_flash.o \
arch/arm/stm32/f10x/stm32f10x_pwr.o \
arch/arm/stm32/f10x/stm32f10x_tim.o \
arch/arm/stm32/f10x/stm32f10x_adc.o \
arch/arm/stm32/f10x/stm32f10x_dac.o \
arch/arm/stm32/f10x/stm32f10x_fsmc.o   \
arch/arm/stm32/f10x/stm32f10x_rcc.o   \
arch/arm/stm32/f10x/stm32f10x_usart.o \
arch/arm/stm32/f10x/stm32f10x_bkp.o  \
arch/arm/stm32/f10x/stm32f10x_dbgmcu.o  \
arch/arm/stm32/f10x/stm32f10x_gpio.o   \
arch/arm/stm32/f10x/stm32f10x_rtc.o   \
arch/arm/stm32/f10x/stm32f10x_wwdg.o \
arch/arm/stm32/f10x/stm32f10x_can.o  \
arch/arm/stm32/f10x/stm32f10x_dma.o    \
arch/arm/stm32/f10x/stm32f10x_i2c.o    \
arch/arm/stm32/f10x/stm32f10x_sdio.o  \
arch/arm/stm32/CMSIS/system_stm32f10x.o \
arch/arm/stm32/CMSIS/startup_stm32f10x_md.o 
