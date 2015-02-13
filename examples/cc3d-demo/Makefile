build-cc3d-demo: build
	$(CXX) $(CXXFLAGS) -fno-exceptions -fno-rtti -c examples/cc3d-demo/main.cpp -o examples/cc3d-demo/main.o
	$(LD) -L$(srctree) $(LDFLAGS) -o examples/cc3d-demo/cc3d-demo -Wl,--start-group \
		 examples/cc3d-demo/main.o -lk-$(BUILD) -lc -lm -lgcc \
	-Wl,--end-group 
	
install-cc3d-demo:
	arm-none-eabi-objcopy -j .text -j .data examples/cc3d-demo/cc3d-demo examples/cc3d-demo/cc3d-demo.bin -O binary
	make -C $(srctree)/scripts/stm32flash/
	arm-none-eabi-size examples/cc3d-demo/cc3d-demo
	sudo $(srctree)/scripts/stm32flash/stm32flash -w examples/cc3d-demo/cc3d-demo.bin -v -g 0x0 /dev/ttyUSB0
