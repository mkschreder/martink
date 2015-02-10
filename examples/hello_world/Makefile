build-hello-world: build
	$(CXX) $(CXXFLAGS) -fno-exceptions -fno-rtti -c examples/hello_world/main.cpp -o examples/hello_world/main.o
	$(LD) -L$(srctree) $(LDFLAGS) -o examples/hello_world/hello-world -Wl,--start-group \
		 examples/hello_world/main.o -lk-$(BUILD) -lc -lm -lgcc \
	-Wl,--end-group 
	
install-hello-world-arm-stm32f103: 
	arm-none-eabi-objcopy -j .text -j .data examples/hello_world/hello-world examples/hello_world/hello-world.bin -O binary
	make -C $(srctree)/scripts/stm32flash/
	sudo $(srctree)/scripts/stm32flash/stm32flash -w examples/hello_world/hello-world.bin -v -g 0x0 /dev/ttyUSB0

install-hello-world-arm-stm32f100mdvl: install-hello-world-arm-stm32f103; 
