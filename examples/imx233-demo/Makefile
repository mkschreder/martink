build-imx233-demo: build
	$(CC) $(CFLAGS) -c examples/imx233-demo/imx233-demo.c -o examples/imx233-demo/imx233-demo.o
	$(CC) -L$(srctree) $(LDFLAGS) -o examples/imx233-demo/imx233-demo -Wl,--start-group \
		 examples/imx233-demo/imx233-demo.o -lk-$(BUILD) -lc -lm -lgcc \
	-Wl,--end-group 
