choice 
	prompt "Hardware type"
	config SIMULATOR
		bool "Compile native for simulator"
	config HARDWARE
		bool "Compile for hardware configured above"
endchoice
