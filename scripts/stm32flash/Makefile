CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
export CC
export AR

CCFLAGS = -Wall
ifeq ($(UNAME_S),Darwin)
CCFLAGS += -mmacosx-version-min=10.6 
endif

all:
	$(MAKE) -C parsers
	$(CC) $(CCFLAGS) -g -o stm32flash -I./ \
		main.c \
		utils.c \
		stm32.c \
		serial_common.c \
		serial_platform.c \
		parsers/parsers.a \
		stm32/stmreset_binary.c \
		-DUSE_SERIAL_RESET

clean:
	$(MAKE) -C parsers clean
	rm -f stm32flash

install: all
	cp stm32flash /usr/local/bin
