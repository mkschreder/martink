Architecture layer
------------------
Martin K. Schröder
info@fortmax.se

When you want to implement support for a new chip, what you would need to do is implement all of the interfaces defined in arch/interface.h file. We will take a look at these interfaces in turn below. But first a little on how things are put together.

This layer consists of following devices:

* [ADC](arch_adc.md) - analog to digital converters
* [DAC](arch_dac.md) - digital to analog converters
* [GPIO](arch_gpio.md) - on chip general purpose io driver
* [PWM](arch_pwm.md) - pulse width modulation peripherals       
* [SPI](arch_spi.md) - on board serial peripheral interface peripherals
* [I2C/TWI](arch_twi.md) - i2c interfaces  
* [UART](arch_uart.md) - asynchronous serial interfaces
* [CAN](arch_can.md) - controller area network interfaces

The arch layer is built around the idea of being really fast. Because this is the absolutely lowest layer that is closest to hardware, the methods of this layer get called a lot of times. This layer is responsible for providing a human readable interface to chip register operations.

We would typically implement this layer by wrapping register operations in macro expressions. You can take a look at how this is done in one of the header files found in for instance arch/avr/m328p folder. All cpu operations are wrapped in macros. You don't have to use macros on higher layers, but this layer is usually very simple (ie all "method" calls in this layer really just write values to some memory location) so we actually want to do this with macros. You can for instance look at the implementation of gpio interface in arch/avr/m328p/gpio.h

Devices and their interfaces
----------------------------

There are typically two ways to access devices on the soc. You can either call the arch layer directly (ie call the macros) or you can use generic interfaces that the arch layer can create for you in order to access the devices generically. Generic interfaces are designed to allow interchangable use of many different types of devices that can provide the same kind of service. For example, libc FILE* handle is a type of generic interface. It has get() and put() function pointers that get called by getc/putc. This way you can use FILE* to access a file on any filesystem, you can access a device, you can access a fifo pipe - all thought exactly the same interface.

This is exactly what we want to have for our device drivers as well!

But there is one problem: the devices that we are working with are all flash based controllers that usually have little memory - both flash and ram. We can not use really rich interfaces like the linux kernel. And we don't want to force the user to use interfaces. We want to provide interfaces simply as an essential feature for enabling driver reuse and for adding a great deal of flexibility for how device drivers are used.

You should think of interfaces as "services" that the device provides as opposed to identifying a specific device. If we make all of our devices "service"-oriented, we can reuse device drivers in many different contexts and pipe data from one driver to another. This is exactly the same philosophy that is used by linux commands where you can create more complex commands easily by simply piping results of one command to the next.

| Interface            | Methods             | Usage                |
|----------------------|---------------------|----------------------|
| analog_interface     | read_pin   | Used for any device that can read an analog signal and report it as 16 bit value. Since many adc peripherals have multiple channels, you specify which channel you want to read. How the data is read is up to the implementation. Your actual driver may for example use adc interrupt and read adc in the background and load it into a cache, making your read_pin call return directly. Others may block until the adc value is converted. 
| parallel_interface   | write_pin, write_word, read_pin, read_word, configure_pin, get_pin_status | GPIO interface is for implementing any type of bit wise parallel io. A lot of device can implement this kind of interface - i2c parallel io chips, shift registers that connect over spi, soc gpio pins, etc. Characteristic of this interface is that it consists of arbitrary number of "pins" that can be written and read, as well as configured to have pullups.
| pwm_interface        | set_channel_duty, set_channel_period | PWM interface is for controlling pwm hardware. Usually it would be built in, timer driven pwm channels, but the system does not limit you to just on board pwm channel You could just as easily have a driver for an i2c or spi pwm controller export this kind of interface and this would allow you to pass it to any component that requires a pwm interface (such as for example a motor speed controller). 
| packet_interface     | begin, end, write, read, sync, packets_available | A packet interface is similar to a serial interface but instead operates on chunks of data. Whereas a serial interface guarantees that you either get or not get an individual byte of data, a packet interface does the same for chunks, meaning that you either get or not get a packet - there is no possibility to only get a partial packet.
| serial_interface     | get, put, getn, putn, flush, waiting | A serial interface is any device that is capable of sending and receiving data over some kind of channel. A serial interface is typically a stream it is not a transaction based interface, but rather a stream based one. Data can usually be transfered in both directions and we do not use any  kind of addressing scheme. A serial interface is a point to point channel between two devices.

Here are some examples of what interfaces different core devices can export:

Click on the device name to read more!

| Device               | Interfaces exported    | 
|:---------------------|------------------------|
| [ADC](arch_adc.md)   | analog_interface    		|
| [GPIO](arch_gpio.md) | parallel_interface   	|
| [PWM](arch_pwm.md)   | pwm_interface         	|
| [SPI](arch_spi.md)   | serial_interface      	|
| [I2C](arch_i2c.md)   | packet_interface       |
| [UART](arch_uart.md) | serial_interface       |
