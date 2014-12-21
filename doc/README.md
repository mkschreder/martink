LibK embedded software library
------------------------------

Libk is more than just a firmware library. It is a toolchain, a philosophy, a way to organize your code. It is maximum flexibility at minimum cost. It is a collection of reusable components for writing bare metal embedded applications \^/

Contents
---------

Architecture specific interfaces and devices:

* [Architecture layer](arch.md)
	* [ADC](arch_adc.md) - analog to digital converters
	* [DAC](arch_dac.md) - digital to analog converters
	* [GPIO](arch_gpio.md) - on chip general purpose io driver
	* [PWM](arch_pwm.md) - pulse width modulation peripherals       
	* [SPI](arch_spi.md) - on board serial peripheral interface peripherals
	* [I2C/TWI](arch_twi.md) - i2c interfaces  
	* [UART](arch_uart.md) - asynchronous serial interfaces
	* [CAN](arch_can.md) - controller area network interfaces

How it works
-------------

Traditional abstraction layers introduce a fair amount of overhead into the application. Of course, any kind of abstraction layer introduces *some* overhead, but this overhead can be minimized. At the lowest level of abstraction, Libk skips traditional function calls altogether and instead uses macros to make peripheral control human readable. Macro based programming forces one to think in terms of simplicity because macros are harder to debug and also a little more cumbersome to implement for the first time. But it pays off in that one is forced to write simple code that only does what is necessary. This allows us to add a basic level of abstraction to core interfaces and make them portable with minimal overhead. 

The only real overhead in libk is in the usage of abstract interfaces (and possibly driver specific buffers). But using interfaces is not mandatory. It is possible to access the lowest level macros and modify chip registers in a human readable (and more portable) way as well. Libk interfaces do however make up for a very powerful collection of components that can be combined together to do more complex functions without having to modify their implementations.

Layers of abstraction
---------------------

Below is an illustration of how the code is structured. Not all documentation is completed yet, so you can click on the links that do work to read more about each layer. 

<table style="text-align: center !important;">
	<tr><th text-align="center" colspan="2">Layers</th></tr>
	<tr><td text-align="center" colspan="2">your application and it's libraries</td></tr>
	<tr><td text-align="center" colspan="2">board driver that implements all necessary services for your application</td> </tr>
	<tr><td text-align="center" colspan="2">reusable, architecture agnostic, device drivers</td></tr>
	<tr><td text-align="center">direct access to macro layer</td><td>generic architecture interface layer</td></tr>
	<tr><td text-align="center" colspan="2"> <a href="arch.md">Architecture specific macro layer</a> </td></tr>
	<tr><td text-align="center" colspan="2"> Memory load and store to peripheral registers</td></tr>
</table>
