VPATH += arch/native

ARCH:=linux
CPU:=generic
BOARD:=native

obj-$(CONFIG_NATIVE) += \
	arch/linux/fd_serial.o \
	arch/linux/time.o \
	arch/linux/i2c_device.o \
	arch/linux/pipe.o \
	arch/linux/spidev.o

LDFLAGS-$(CONFIG_NATIVE) += -lpthread
