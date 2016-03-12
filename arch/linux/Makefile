VPATH += arch/native

ARCH:=linux
CPU:=generic
BOARD:=native

obj-$(CONFIG_ARCH_LINUX) += \
	arch/linux/fd_serial.o \
	arch/linux/time.o \
	arch/linux/i2c_device.o \
	arch/linux/pipe.o 

CFLAGS-$(CONFIG_ARCH_LINUX) += -Wno-unused-function -Wno-unused-variable
LDFLAGS-$(CONFIG_ARCH_LINUX) += -lpthread
