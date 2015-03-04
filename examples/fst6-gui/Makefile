build-fst6-gui: build
	$(CXX) $(CXXFLAGS) -fno-exceptions -fno-rtti -c examples/fst6-gui/main.cpp -o examples/fst6-gui/main.o
	$(LD) -L$(srctree) $(LDFLAGS) -o examples/fst6-gui/fst6-gui -Wl,--start-group \
		 examples/fst6-gui/main.o -lk-$(BUILD) -lc -lm -lgcc \
	-Wl,--end-group 
	
install-fst6-gui:
	arm-none-eabi-objcopy -j .text -j .data examples/fst6-gui/fst6-gui examples/fst6-gui/fst6-gui.bin -O binary
	make -C $(srctree)/scripts/stm32flash/
	arm-none-eabi-size examples/fst6-gui/fst6-gui
	sudo $(srctree)/scripts/stm32flash/stm32flash -w examples/fst6-gui/fst6-gui.bin -v -g 0x0 /dev/ttyUSB0
