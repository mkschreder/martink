LibK embedded software library
------------------------------

Libk is more than just a firmware library. It is a toolchain, a philosophy, a way to organize your code. It is maximum flexibility at minimum cost. It is a collection of reusable components for writing bare metal embedded applications \^/

Contents
---------

Architecture specific interfaces and devices
* [Architecture layer](arch.md)
** [ADC](arch_adc.md) - analog to digital converters
** [DAC](arch_dac.md) - digital to analog converters
** [GPIO](arch_gpio.md) - on chip general purpose io driver
** [PWM](arch_pwm.md) - pulse width modulation peripherals       
** [SPI](arch_spi.md) - on board serial peripheral interface peripherals
** [I2C/TWI](arch_twi.md) - i2c interfaces  
** [UART](arch_uart.md) - asynchronous serial interfaces
** [CAN](arch_can.md) - controller area network interfaces

How it works
-------------

Traditional abstraction layers introduce a fair amount of overhead into the application. Of course, any kind of abstraction layer introduces *some* overhead, but this overhead can be minimized. At the lowest level of abstraction, Libk skips traditional function calls altogether and instead uses macros to make peripheral control human readable. Macro based programming forces one to think in terms of simplicity because macros are harder to debug and also a little more cumbersome to implement for the first time. But it pays off in that one is forced to write simple code that only does what is necessary. This allows us to add a basic level of abstraction to core interfaces and make them portable with minimal overhead. 

The only real overhead in libk is in the usage of abstract interfaces (and possibly driver specific buffers). But using interfaces is not mandatory. It is possible to access the lowest level macros and modify chip registers in a human readable (and more portable) way as well. Libk interfaces do however make up for a very powerful collection of components that can be combined together to do more complex functions without having to modify their implementations.

Layers of abstraction
---------------------

Below is an illustration of how the code is structured. Not all documentation is completed yet, so you can click on the links that do work to read more about each layer. 

<table text-align="center">
	<tr><th colspan="2">Layers</th></tr>
	<tr><td colspan="2">your application and it's libraries</td></tr>
	<tr><td colspan="2">board driver that implements all necessary services for your application</td> </tr>
	<tr><td colspan="2">reusable, architecture agnostic, device drivers</td></tr>
	<tr><td>direct access to macro layer</td><td>generic architecture interface layer</td></tr>
	<tr><td colspan="2"> <a href="arch.md">Architecture specific macro layer</a> </td></tr>
	<tr><td colspan="2"> Memory load and store to peripheral registers</td></tr>
</table>

LibK license?
-------------

LibK is licensed under GPLv3. But with a few clarifications.

First of all, just like on linux, for maximum flexibility we need you to feel that it's ok for you to build software that makes system calls into LibK without having to release source code to that software. But it should also be clear that any extensions or contributions to LibK must comply with GPLv3 and be open source. Therefore a few clarifications have been added. 

- You may build software on top of LibK without having to release source code to that software as long as:
	- all calls made to libk from your program only go one way into libk without any code from any of libk modules calling any callbacks that reside in your main application. Code that does this usually implements some device driver or an interface anyway, and thus belongs in libk itself and should thus be submitted to the main repository. The situation of libk calling back into your code occurs for instance when you write a custom device driver that exports one of libk's standard interfaces and then you pass that interface to another driver within libk. If you want to use LibK in this way, you need to either comply with GPLv3 and make it open source or purchase consulting from us and get code for free. For contributions to libk made by others, special rules may apply (ie that some code may only be used in open source context). To be really sure, follow guidelines listed below. 
- if you write code that implements device support, you should submit it into the main tree. 
- if you make any changes or improvements to link, you should submit pull request to get it merged into main tree as well. 
- you can not take code from libk and make it part of an exclusively closed source project. 
- you can purchase consulting and you will get code for free. Send a request to info@fortmax.se.
- all code within libk, and all code that it calls directly or through a callback is released without warranty and under the terms of gnu public license. 

To achieve the best balance between project interests and your own business interests, here is what you can do:

- implement support for your hardware as a board driver, submit this driver into the public tree.
- if you find open source code, manufacturer code, or any other library or your own code that implements hardware support for some sort of device that you want to use on your board, submit it also to the public tree. 
- write your application so that it accesses all required services through the board driver which you have open sourced earlier (or which probably already was open source code when you started anyway - due to it usually being released as such from the manufacturer). Following these guidelines, you can keep your actual application safely separated from libk and use any licensing terms you want for code that belongs strictly to your specific project.

Fortmax reserve the right to make any changes to these terms as we deem necessary in order to achive the best possible balance between the open source nature of the project and your need to keep some parts of your application private. 

