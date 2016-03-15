obj-y += arch/time.o
obj-y += arch/soc.o

docs-y += arch/README.md

ifeq ($(CONFIG_ARCH_AVR),y)
	include $(srctree)/arch/avr/Makefile
endif

ifeq ($(CONFIG_ARCH_LINUX),y)
	include $(srctree)/arch/linux/Makefile
endif

ifeq ($(CONFIG_ARCH_ARM),y)
	include $(srctree)/arch/arm/Makefile
endif

ifeq ($(CONFIG_ARCH_XTENSA),y)
	include $(srctree)/arch/xtensa/Makefile
endif

