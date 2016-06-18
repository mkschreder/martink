obj-y += kernel/main.o
obj-y += kernel/thread/async.o
obj-y += kernel/thread/process.o
obj-y += kernel/cbuf.o
obj-y += kernel/gbuf.o
obj-y += kernel/sbuf.o 
obj-$(CONFIG_FREERTOS) += kernel/mt.o 
obj-y += kernel/avl.o
obj-y += kernel/avl-cmp.o
obj-y += kernel/device.o

docs-y += kernel/README.md

include kernel/dev/Makefile
include kernel/freertos/Makefile
