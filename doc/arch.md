Architecture layer
------------------

When you want to implement support for a new chip, what you would need to do is implement all of the interfaces defined in arch/interface.h file. We will take a look at these interfaces in turn below. But first a little on how things are put together.

This layer consists of following devices:

* [ADC](arch_adc.md) 
* [GPIO](arch_gpio.md)  
* [PWM](arch_pwm.md)         
* [SPI](arch_spi.md)      
* [I2C/TWI](arch_twi.md)       
* [UART](arch_uart.md)
* CAN

The arch layer is built around the idea of being really fast. Because this is the absolutely lowest layer that is closest to hardware, the methods of this layer get called a lot of times. This layer is responsible for providing a human readable interface to chip register operations.

We would typically implement this layer by wrapping register operations in macro expressions. You can take a look at how this is done in one of the header files found in for instance arch/avr/m328p folder. All cpu operations are wrapped in macros. You don't have to use macros on higher layers, but this layer is usually very simple (ie all "method" calls in this layer really just write values to some memory location) so we actually want to do this with macros. You can for instance look at the implementation of gpio interface in arch/avr/m328p/gpio.h

Devices and their interfaces
----------------------------

There are typically two ways to access devices on the soc. You can either call the arch layer directly (ie call the macros) or you can use generic interfaces that the arch layer can create for you in order to access the devices generically. Generic interfaces are designed to allow interchangable use of many different types of devices that can provide the same kind of service. For example, libc FILE* handle is a type of generic interface. It has get() and put() function pointers that get called by getc/putc. This way you can use FILE* to access a file on any filesystem, you can access a device, you can access a fifo pipe - all thought exactly the same interface.

This is exactly what we want to have for our device drivers as well!

But there is one problem: the devices that we are working with are all flash based controllers that usually have little memory - both flash and ram. We can not use really rich interfaces like the linux kernel. And we don't want to force the user to use interfaces. We want to provide interfaces simply as an essential feature for enabling driver reuse and for adding a great deal of flexibility for how device drivers are used.

You should think of interfaces as "services" that the device provides as opposed to identifying a specific device. If we make all of our devices "service"-oriented, we can reuse device drivers in many different contexts and pipe data from one driver to another. This is exactly the same philosophy that is used by linux commands where you can create more complex commands easily by simply piping results of one command to the next.

| Device               | Interface              |
|:---------------------|------------------------|
| ADC                  | [analog_interface](arch_adc.md)       |
| GPIO                 | [parallel_interface](arch_gpio.md)     |
| PWM                  | [pwm_interface](arch_pwm.md)         |
| SPI                  | [serial_interface](arch_spi.md)      |
| I2C                  | [packet_interface](arch_twi.md)       |
| UART                 | [serial_interface](arch_uart.md)       |
