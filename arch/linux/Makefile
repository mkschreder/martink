VPATH += arch/native

obj-$(CONFIG_NATIVE) += \
	arch/linux/fd_serial.o \
	arch/linux/native.o \
	arch/linux/time.o \
	arch/linux/i2c_device.o \
	arch/linux/pipe.o

LDFLAGS-$(CONFIG_NATIVE) += -lpthread
