build-hello-world: build
	$(CXX) $(CXXFLAGS) -c examples/hello_world/main.cpp -o examples/hello_world/main.o
	$(LD) -L$(srctree) $(LDFLAGS) -o examples/hello_world/hello-world -Wl,--start-group \
		-lk-arm-stm32f103 -lc -lm -lgcc examples/hello_world/main.o \
	-Wl,--end-group 
	
install-hello-world: 
	arm-none-eabi-objcopy -j .text examples/hello_world/hello-world examples/hello_world/hello-world.bin -O binary
	make -C $(srctree)/scripts/stm32flash/
	sudo $(srctree)/scripts/stm32flash/stm32flash -w examples/hello_world/hello-world.bin -v -g 0x0 /dev/ttyUSB0
