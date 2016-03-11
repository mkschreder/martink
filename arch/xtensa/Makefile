ARCH:=xtensa
CPU:=esp8266
BOARD:=generic
CC:=xtensa-lx106-elf-gcc

obj-$(CONFIG_XTENSA_APP_BLINKY) += arch/xtensa/blinky.o

CFLAGS-$(CONFIG_XTENSA) += -mlongcalls -Wno-implicit-function-declaration -Wno-unused-function
LDFLAGS-$(CONFIG_XTENSA) += -nostdlib -Wl,--start-group -lmain -lnet80211 -lwpa -llwip -lpp -lphy -Wl,--end-group -lgcc -Teagle.app.v6.ld
