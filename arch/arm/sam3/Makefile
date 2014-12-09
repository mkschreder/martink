
CC := arm-none-eabi-gcc
AS := arm-none-eabi-as
CXX := arm-none-eabi-g++
LD := arm-none-eabi-ld
LDXX := arm-none-eabi-g++

curdir := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

#-L$(curdir)/../../../lib/newlib-2.1.0/bin/arm-none-eabi/lib/ \

LDFLAGS += \
-T$(curdir)/sam3flash.ld\
-Wl,-Map,$(APPNAME).map \
-nostdlib -mthumb -Wl,--cref -Wl,--check-sections -Wl,--gc-sections \
-Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common \
-Wl,--warn-section-align  \

EXTRALIBSS += $(curdir)/../../../lib/newlib-2.1.0/bin/arm-none-eabi/lib/libm.a \
$(curdir)/../../../lib/newlib-2.1.0/bin/arm-none-eabi/lib/libc.a \
$(curdir)/../../../lib/newlib-2.1.0/bin/arm-none-eabi/lib/libg.a \

CPU_FLAGS := 

COMMON_FLAGS += -DF_CPU=84000000L -DARDUINO=10600 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON -DUSB_MANUFACTURER="Unknown" -DUSB_PRODUCT="Arduino Due" -I$(curdir)/CMSIS/sam/sam3xa/include/ -I$(curdir)/CMSIS/ -I$(curdir)/../../../lib/newlib-2.1.0/newlib/libc/include/
#arm-none-eabi/include/ -I$(curdir)/../arm-none-eabi/gcc/arm-none-eabi/4.8.3/include/

COMMON_FLAGS += --param max-inline-insns-single=500 -mcpu=cortex-m3 -mthumb -mlong-calls -ffunction-sections -fdata-sections -std=c99

ASFLAGS = -mcpu=cortex-m3 -mthumb -Wall -a -g $(INCLUDES)

obj-y += arch/arm/sam3/uotghs_host.o arch/arm/sam3/pmc.o
obj-y += arch/arm/sam3/util.o \
arch/arm/sam3/syscalls_sam3.o \
arch/arm/sam3/adc12_sam3u.o  	arch/arm/sam3/efc.o   	arch/arm/sam3/pio.o   arch/arm/sam3/rtc.o  arch/arm/sam3/tc.o   	arch/arm/sam3/udp.o \
arch/arm/sam3/adc.o     			arch/arm/sam3/emac.o    arch/arm/sam3/rtt.o 		arch/arm/sam3/timetick.o  \
arch/arm/sam3/udphs.o         arch/arm/sam3/usart.o		arch/arm/sam3/can.o   arch/arm/sam3/gpbr.o arch/arm/sam3/pwmc.o  	arch/arm/sam3/spi.o  \
arch/arm/sam3/trng.o      		arch/arm/sam3/uotghs.o  arch/arm/sam3/wdt.o		arch/arm/sam3/dacc.o arch/arm/sam3/interrupt_sam_nvic.o  \
arch/arm/sam3/rstc.o  				arch/arm/sam3/ssc.o  		arch/arm/sam3/twi.o   arch/arm/sam3/uotghs_device.o \
arch/arm/sam3/CMSIS/sam/sam3xa/source/gcc_arm/startup_sam3xa.o \
arch/arm/sam3/CMSIS/sam/sam3xa/source/system_sam3xa.o
