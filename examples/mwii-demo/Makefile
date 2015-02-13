build-mwii-demo: build
	$(CXX) $(CXXFLAGS) -fno-exceptions -fno-rtti -c examples/mwii-demo/main.cpp -o examples/mwii-demo/main.o
	$(LD) -L$(srctree) $(LDFLAGS) -o examples/mwii-demo/mwii-demo -Wl,--start-group \
		 examples/mwii-demo/main.o -lk-$(BUILD) -lc -lm -lgcc \
	-Wl,--end-group 
	
install-mwii-demo:
	avr-objcopy -j .text -j .data -O ihex examples/mwii-demo/mwii-demo examples/mwii-demo/mwii-demo.hex
	avr-size examples/mwii-demo/mwii-demo
	sudo avrdude -p m328p -c usbasp -e -U lfuse:w:0xFF:m -U hfuse:w:0xD7:m -U efuse:w:0x05:m -U flash:w:examples/mwii-demo/mwii-demo.hex
