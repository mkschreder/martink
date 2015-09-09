build-native-demo: build
	$(CC) $(CFLAGS) -c examples/native-demo/main.c -o examples/native-demo/main.o
	$(CC) -L$(srctree) $(LDFLAGS) -o examples/native-demo/native-demo -Wl,--start-group \
		 examples/native-demo/main.o -lk-$(BUILD) -lc -lm -lgcc  \
	-Wl,--end-group 
