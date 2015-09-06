Introduction
===============

LibK is a lightweight firmware development library that is designed for flash based, resource constrained microcontrollers that are not able to run a full modern operating system. LibK uses protothread-based, cooperative multitasking to make all I/O operatins non-blocking and to make the most of the already constrained CPU by never wasting any clock cycles waiting for an I/O operation to complete. 

Included in LibK you will find many device drivers that make it possible to use the supported devices in your project directly without having to implement the device drivers yourself. The device drivers have also been rewritten by me to use cooperative multitasking so that no device can ever slow down your application just because it is slow. 

Using LibK you can build portable applications for AVR-8, STM32 and PC. 

Gettings started: compiling and flashing
----------------------------------------

LibK build system is originally designed for building on Ubuntu linux so it may require modifications if you want to build on a different system. The build process uses Makefiles that it includes from all of the subfolders. Each subfolder has it's own Makefile, KConfig file and a README.md (docs) file. All of these files are aggregated by make and then compiled into their respective targets. 

To compile LibK you will also need appropriate gcc toolchain for your hardware target. 

* avr-gcc and avr-libc for compiling for atmega, arduino, avr32
* arm-none-eabi-gcc for compiling for sam3, stm32 and other arm architectures
* pandoc for compiling this PDF from all readme files (although you should have this PDF included)
* clang (scan-build) for enabling static analysis of the code during the build
* lib-curses for compiling menuconfig (for editing libk configurations)
* avrdude for flashing avr chips

On Ubuntu: 

	sudo apt-get install gcc-avr gcc-arm-none-eabi avr-libc pandoc libncurses5-dev avrdude

Build everything: 

	make buildall

This will build all supported architectures, produce corresponding libraries and compile examples as well. 

The above will in fact invoke build commands for each target. For building a specific target library use BUILD variable that you pass to MAKE like this: 

	make BUILD=arm-stm32f103
	
The parameter to this command should be the same string as the config file name of the corresponding config in the "configs" folder. The build script will then include appropriate source files relevant only for that target and compile them into a corresponding static library. 

Installing on your hardware
--------------------------

Note: Depending on how your target flashing process works, these instructions may not apply to you. In that case, find out how to actually burn a binary file on your architecture from the documentation of your target device. 

All libk programs compile to binary elf files that are then converted to either a binary blob or a hex file using appropriate objdump utility for the target platform. This resulting hex or bin file can then be directly passed to the flashing software and transferred to the target chip. Usually this transfer is done over a usb to serial converter. 

* Avr platform uses tool avrdude and expects avr to be connected to usbasp programmer. For flashing using a different programmer you will need to modify the makefiles (in examples folders). 
* STM32 uses builtin ST bootloader and the stm32flash utility. You will need to explicitly boot the chip into the bootloader mode by holding boot0 low at poweron. Some firmwares also support booting into the bootloader from software (check the fst6-demo example for this can be done. It implements a firmware upgrade command). 

When building your own software, you will usually link libk into your project and then flash using your standard flashing process. For the examples provided inside libk, there are however makefile targets specified for flashing each of the examples. All examples are built using "make BUILD=(target) build-(example name)" pattern and installed using the corresponding "make BUILD=(target) build-(example name)" pattern. 

	make BUILD=arm-stm32f100mdvl build-fst6-demo
	make BUILD=arm-stm32f100mdvl install-fst6-demo

Note: if you get a "permission denied" when trying to access your serial port as normal user in linux, then make sure to add yourself to the "dialout" group. 

<!--
More information
---------------

For documentation check out the following pages:

* [Detailed hardware and api documentation for libk](https://github.com/mkschreder/martink/tree/master/doc/)

It is distributed under the GNU General Public License - see the accompanying LICENSE.txt file for more details.

It is designed to even be able to compile on a desktop. The future may very well involve using libk to use the same device drivers for both arduino and a linux based board that uses linux to access i2c ports and other ports. LibK can basically be compiled as a native library. All that is necessary to run it on any board is to implement the arch interface for that board. For a native version this layer would use linux system calls to access i2c or SPI ports instead of accessing hardware registers directly - but for the drivers it does not matter.

It is howerver primarily designed for microcontrollers, flash based system on chip devices, small controllers with under 10kb of flash, controllers with as little as 2kb of ram, arm chips and avr chips. For many small devices linux simply does not work. What you wish you had is a library that makes it easy for you to write reusable code and abstracts away the absolutely lowest levels of hardware interaction with minimal overhead. 

Do you ever have problems with figuring out how to use 'UDSR' or 'UCR' or 'BCC' or any other obscure register just to get some piece of hardware configured correctly? Do you ever always have datasheet open for the cpu you are working on just because you can't remember which bits in what registers you need to set?

Yes unfortunately there has not been a good alternative to linux speciffically designed for tiny microcontrollers. This has resulted in many people designing their own solutions that are often completely incompatible when you find code from somebody else for another on chip peripheral. 

LibK provides a framework for solving all of these problems. It comes with specialized build system based on menuconfig that provides a highly fine grained build configuration and lets you only compile the things that you need. It also makes it easy to switch between different targets that require completely different low level implementations.

Building with menuconfig
--------------

** note ** 
Docs need to be updated here.. manuconfig can still be used, but preconfigured configs are also available in "configs/" and "include/configs" directories. This makes it not necessary to run menuconfig unless one wants to have more detailed control over what is actually being built. 
** end not **

Building libk can be a little tricky for a first timer. It is usually easier to call libk Makefile from within your own application Makefile. For how to do this, you can have a look at quadcopter example project using libk: 

[https://github.com/mkschreder/bettercopter](https://github.com/mkschreder/bettercopter)

To build with LibK you need to make sure that you have necessary toolchain installed for your architecture. For avr it's avr-gcc, for arm it's either arm-none-eabi-* or arm-linux-eabi-* packages. You also need libncurses-dev (for menuconfig). 

New: it is now possible to easily build for multiple targets using BUILD option. 

Currently available options: 
	BUILD=avr-atmega328p - build for atmega
	BUILD=arm-stm32f103 - build for stm32f103

You can then build the library like this: 

    make menuconfig
    make BUILD=arm-stm32f103 
    make BUILD=arm-stm32f103 install

If everything goes well, the file that will be built will be libk.a. This is a library that you will then link your application with. It is often easier to integrate the build process for libk into the build process for your project. It is also good to make a symbolik link to libk code directory directly inside your project directory. This way you don't have to keep track of multiple copies of libk.
-->

Supported architectures
------------------

| Manufacturer | Chip | status |
|--------------|------|--------|
| Atmel | ATMega328p | my primary focus right now |
| Atmel | AT91SAM3 ARM | my secondary focus but now yet fully supported |
| ST | STM32F103 | peripheral library is included, but device interfaces not implemented yet |

Device driver support
---------------------

Device drivers in libk operate on a higher level than architecture code. So they are largely architecture agnostic. Device drivers typically use interfaces to access services provided by the architecture and they can also export interfaces to other drivers in order to eliminate dependencies between drivers on each other.

| Device class | Device model | Support | Interfaces used | Interfaces exported |
|--------------|--------------|---------|-----------------|---------------------| 
| Board | Multiwii V2.5 | Supported | `flight_controller` interface | |
| Board | Arduino Pro Mini | Supported | Architecture ATMega328p | |
| Board | Stm32f103 development board | planned | | |
| Board | Crius ATmega256 flight control | planned | |  |
| Crypto | AES 256 | Supported |  | |
| Display | ILI9340 | Supported | parallel_interface and serial_interface | |
| Display | Parallel LCD | Supported | parallel_interface | |
| Display | 7 segment led | Supported | parallel_interface | |
| Display | 8x8 Led matrix | Supported | parallel_interface | |
| Display | ssd1306 OLED | Supported | i2c_interface | |
| Filesystem |  | Planned | | |
| HID | WiiNunchuck | Supported | i2c_interface | |
| IO | 74HC165 | Supported | serial_interface, parallel_interface | |
| IO | 74HC4051 | Supported | parallel_interface | |
| IO | 74HC595 | Supported | serial_interface, parallel_interface | |
| IO | PCF8574 | Supported | i2c_interface | planned: parallel_interface |
| NET | ENC28J60 Supported | serial_interface | planned: i2c_interface |
| NET | TCPIP | Supported | i2c_interface | planned: serial_interface |
| RADIO | NRF24L01 | Supported | serial_interface | planned: i2c_interface |
| SENSOR | ACS712 | Supported | analog_interface | |
| SENSOR | ADXL345 | Supported | i2c_interface | |
| SENSOR | AMT345 | Supported | analog_interface | |
| SENSOR | BH1750 | Supported | i2c_interface | |
| SENSOR | BMP085 | Supported | i2c_interface | |
| SENSOR | DHT11 | Supported | parallel_interface | |
| SENSOR | DS18B20 | Supported | parallel_interface | |
| SENSOR | FS300A | Supported | | |
| SENSOR | HCSR04 | Supported | parallel_interface | |
| SENSOR | HMC5883L | Supported | i2c_interface | |
| SENSOR | L3G4200D | Supported | i2c_interface | |
| SENSOR | LDR | Supported | analog_interface | |
| SENSOR | MMA7455 | Supported | i2c_interface | |
| SENSOR | MPU6050 | Supported | i2c_interface| |
| SENSOR | NTCTEMP | Supported | analog_interface | |
| SENSOR | TSL235 | Supported | | |
| TERMINAL | VT100 | Supported | framebuffer_interface | serial_interface |

Drivers are tested on avr. Other architectures may not have full drivers support until there is stable architecture abstraction layer written for that architecture. 

(devices marked as quarantine are drivers that have not been updated yet after changes to the core api. Code has been included in the source tree but it has not yet been updated to work nicely with other facilities of libk)

Design goals
------------

- To maintain small memory footprint and use minimal amount of ram
- To take advantage of as much of the (limited) CPU power as possible 
- To maintain high portability across all of the supported platforms
- To minimize the amount of code inside the application that needs to be changed to support a new platform. 

How the design goals are currently implemented: 

- LibK places as much constant data as possible into flash and thus frees up a lot of ram for use in the application
- LibK uses cooperative multitasking inside the kernel and all drivers are written to never block the cpu while waiting for an external event or I/O. 
- LibK uses asynchronous I/O at all times, meaning that no read or write operation is ever blocking. This allows for other tasks to happen while a task is waiting for io. 
- LibK uses protothreads instead of conventional threads - this means that threads never have any stack and all threads execute inside the same stack space of the main application. It also have many other advantages that are discussed further in this document. 

<!--
Architechture
=============

The system uses makefiles for building the projects. This is a good thing because makefiles are extremely powerful and allow for very specialized compilation process. Through use of separate makefiles in different parts of the project we can specify exactly which files are compiled and included. We can also load configuration from menuconfig directly into a makefile and write rules based on the config. This build system is highly influenced by the very efficient build system used by linux kernel. 

We use mconf utility for generating the .config file which is included in makefiles and autoconf.h file which can be included into the application. Mconf (menuconfig) allows us to specify dependencies between different modules of the project and allow for highly specialized conditional compilation of different files for different systems. 

The final result produced by the system is a built-in.o file that contains all the code for the main kernel. This file is then linked into your project to allow it to access code that talks to different peripherals such as on chip UART or SPI or I2C or any other supported device. 

Many boards can be supported, by providing board speciffic files in the "boards" directory. Each file correspond to a supported board. Your application should do things like setting PWM output values by calling set_pin() method that is defined for every board. This method can do much more than just setting a GPIO pin. It can also take an integer value specifying PWM level and use it to fill appropriate hardware registers to generate output. Your board file should be speciffically designed for every board and it should provide an interface for the rest of the application for accessing these board peripherals. 

Many drivers for out of chip devices (such as I2C peripherals) are completely reusable on many different architectures. For this reason, we group all architecture speciffic files into arch/ directory. The purpose of the architecture speciffic files is to provide an interface for the DRIVERS which they can use to access standard peripherals such as I2C or SPI busses. The arch directory should therefore only contain files that implement ON CHIP peripherals that the IC uses to communicate with the outside world. 

When arch layer is ported to a different SoC, it should be possible to recompile the kernel and run all other drivers on that platform without any other changes.

Known issues
------------

The way that devices are currently accessed is by using a prefix: ie uart_init() or spi_readwritebyte(). This may become an issue when we begin supporting boards with several uarts. This requires some thinking because it is not possible to use fancy things like dynamically allocated arrays of function pointers that would make every device completely interchangable. All devices in the kernel are sortof hardwired once it is compiled. This allows for very tiny memory footprint and allows the code to run well on small ICs with very little ram. Probably this can be partly solved by simply implementing uart1_x() groups of methods that would allow accesing the uart1 instead of uart0. This is how it has been done in many other microcontroller projects. Options can then be added into menuconfig to allow some higher level peripheral to be configured to use uart1 instead of uart0 and basically the problem may be solved. 

Another issue: drivers need to be revised and fixed so that there are no internal conflicts. Many drivers come from projects where it has been decided that the driver would have exclusive access to some peripheral. But when we have several devices, exclusive access is not possible. So one driver may set a register and another driver may accidentally change it. For the most part this has been fixed, but there are some drivers for components that I have not been able to test. So compile, test, if it works fine, if it doesn't, it should be fairly easy to fix. Then submit a patch so we can get it fixed as well.

Development tools 
-----------------

For reasons that using makefiles and utilities that are otherwise standard on linux is hard on other systems, it is recommended that you use linux (or maybe macos) for working on this project. It is simply much easier to write a powerful build system like menuconfig on linux than it is on windows. For compilation use GCC, for scripting use bash, for editing use whatever editor you like. If you are on windows then simply run a version of Ubuntu in VirtualBox and map files from your host system into the emulated linux and build as usual while still using windows for other things. 
-->
Kernel Architecture
===================

LibK is a lightweight kernel that implements cooperative multitasking using stackless threads. Stackless threads have been chosen because they are the most lightweight kind of threads available and they have very small overhead. There are however both advantages and also some disadvantages to using stackless protothreads. 

Advantages of stackless protothreads: 

- Allow device drivers to be written such that no CPU cycles are ever wasted waiting for I/O. The device driver can simply return control to the application and wait until the next time it has the chance to run. 
- Writing asynchronous tasks becomes a lot easier because they can be written linearly instead of being organized as a complicated state machine. 
- No thread is ever interrupted while it is in the middle of some operation. Threads always run to completion. Meaning that we can design our code without having to think about byte level synchronization (device level syncrhonization is required though). 
- No stack also means that we never run out of stack space inside a thread. 
- Context switching is very fast - current thread method saves it's resume point, returns to libk scheduler, libk scheduler loads the address of the next thread function to run, and calls it. 
- Not a problem to have tens of threads for each asynchronous action. Since threads are just normal methods minus the stack, we can have many threads without experiencing significant slowdown. 
- Synchronization is much easier because all code that you "see" is atomic until it explicitly releases control to the scheduler. We thus do not have to worry about non atomic memory access. 

Disadvantages of this approach: 

- Stackless means that no variable on the thread method stack is valid after a thread returns and then resumes again. Although we can easily solve this by maintaining the context inside a separate object to which the thread is attached. 
- No preemption also means that a thread can keep CPU to itself for as long as it wishes. It is up to the programmer to release the CPU as quickly as possible. Since device drivers usually use interrupt requests to respond in realtime anyway, this limitation has not proven to be a problem so far. 
- No thread can ever call another thread or spawn a new thread. Threads are only single level. All other code called from inside the thread can be considered to execute atomically (except for when it is interrupted by an ISR). 
- Data that needs to be saved across multiple thread switches must be stored in memory (this is a problem with all multitasking though). 
- Longer response times for tasks - a task can lose cpu for as long as it takes the heavies task to finish. This is solved by programmer explicitly designing threads such that control is periodically released to the scheduler. 

A detailed evaluation of libk threading
--------------------------------------

Stackless threads in libk are designed to solve one specific problem: busy waits. I settled for this approach because it has been the most lightweight solution to this problem. Almost 98% of all embedded code uses busy waits - the standard way to solve this problem is by implementing a scheduler that is able to interrupt a currently running task in the middle of it's busy loop and switch to a different task. This however also comes with a lot of subtle problems that result in much more synchronization code all around the application to ensure atomic access to shared data. 

With protothreads, it is instead possible to minimize the amount of locking and synchronization necessary, while at the same time to enjoy a healthy degree of multitasking where CPU rarely is just idly spinning inside some delay loop.

The main area where this kind of multitasking really is useful is device drivers that do a lot of I/O. I/O operations are by far the greatest bottleneck in most embedded systems that don't use multitasking and instead resort to idly waiting for an I/O operation to complete. LibK solves this problem by doing minimal caching of data and also by never waiting for an I/O operation to complete and instead letting another task run while I/O is in progress. 

Another attractive feature of libk threading is that it is completely implemented in software - meaning that it will work the same on all hardware. It is after all just an array of "update" methods that the kernel schedules periodically. 

I have found that I could improve performance with protothreading almost 100x. When I eliminated all busy delay loops in the device drivers I have found that my application was able to run a lot faster and also it has become much more responsive. I have not howerver noticed a significant memory overhead. By far the main memory overhead (which is also a necessary evil) is caching data in memory so that it can be retained while another thread has control of the CPU. Most drivers use caching in one way or another. 
Device interfaces
=================


How to device access works
--------------------------

For the sake of being light weight, the on chip devices all have static methods that are used for accessing each device. You can use macros to construct code for accessing a low level on chip device and most of these macros are device in arch/*.h files.

Thus we can write spi0_init() and spi0_writereadbyte() to interact with the spi0 interface on the chip. All this is fine, until we want to configure another device driver to use a speciffic spi device. Of course we could write the driver so that it has the spi device hardcoded and always uses one speciffic output device - this is fine if we only have one instance of the driver - but boards can have several instances of the same device connected to completely different spi ports. And this needs to be configured.

This does require some kind of way to reference an on chip device that can be specified dynamically in code. Generating separate code for each instance of the device no longer provides much benefit - so we need to use some extra memory to do this. The way this is solved right now is through the use of interfaces. Every type of low level device has an interface struct that can be optionally used to access that device from a driver. This interface we can construct dynamically. The overhead is a little memory used for storing the function pointers and a small overhead of calling a function pointer but this is negligible unless you have some really really small device. And also the library allows you to actually not use this interface at all if you don't want to - however higher level drivers use it because it adds a configurability benefit to the driver so that we can easily define multiple instances of a driver for a board.

All device instance data is stored in a device specific struct along with any interfaces to lower level devices that the driver uses to talk to the device. This way we can configure a device in the board file to use i2c port X and address Y to talk to a device and then the driver code can use the constructed interface to access the device without knowing exactly which i2c port and address it is using. The driver does not need this data - it only needs a reference to an i2c interface that is already preconfigured to use specific settings for that board.

The kind of code structure we are aiming to achieve is that all on board devices have to be configured in the board driver. All application code then uses structs defined in the board struct to talk to all devices accessible to it. 
 
Architecture layer
==================

- Martin K. Schröder - info@fortmax.se

Currently supported architectures
-----------------------

| VENDOR   |    Model      |Status |
|----------|:-------------:|------:|
|   ATMEL  |  ATMega328p   |  yes  |
|   ATMEL  |   AT91SAM3    |  yes  |
|   ST     |   STM32F103   |  part |

When you want to implement support for a new chip, what you would need to do is implement all of the interfaces defined in arch/interface.h file. We will take a look at these interfaces in turn below. But first a little on how things are put together.

The whole arch layer for an architecture can be included into your application by including arch/soc.h file. It will automatically include needed files according to include/autoconfig.h file (generated by 'make menuconfig')

```
#include <arch/soc.h>
```

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

Analog to digital converters
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/adc.h
* arch/adc.h

These devices are expected to provide a service of reading an analog signal on one of available pins and reporting the value as a digital 16 bit number. Adc implementation is limited to 256 channels (although can easily be extended in the future if one needs more). An adc driver should typically only control one type of device or a few types of devices that are very similar to each other. If a device differs too much, a separate driver should be created.

Public interface
----------------

| struct analog_interface method | Description |
|--------------------------------|-------------|
int16_t (*read_pin)(adc, pin_number) | reads adc pin and returns it's integer representation. 

Implementation macro guidelines
----------------------

| Method   | Description |
|----------|-------------|
| adc0_enable() | provides means to turn on the adc peripheral
| adc0_disable() | will turn off adc to save power
| adc0_init_default() | configures adc to default settings (typically internal vref, left alignment and slowest possible clock for highest accuracy)
| adc0_set_vref(adcref) | must provide means of setting voltage reference source for the conversion.
| adc0_set_prescaler(adc_clock) | sets adc clock prescaler 
| adc0_set_alignment(adc_align) | must provide means of setting alignment of result (left or right)
| adc0_interrupt_on() | turns on adc interrupt 
| adc0_interrupt_off() | turns off adc interrupt
| adc0_set_channel(adc_chan) | provides means to tell adc peripheral which channel to sample
| adc0_get_channel() | evaluates to whatever channel we are currently sampling
| adc0_start_conversion() | kicks off adc conversion
| adc0_conversion_in_progress() | checks if a conversion is currently in provress
| adc0_wait_for_completed_conversion() | returns when current conversion has finished
| adc0_read() (uint16_t) | simply waits for previous conversion to complete first and then returns sampled value for currently selected channel. 
| adc0_read_immediate(chan) | kicks off a conversion, waits for it to complete and returns the value for supplied channel. 
| adc0_set_mode(adc_mode) | provides access for setting different adc modes (such as manual or automatic continuous mode where data is sampled all the time and cached for future use. Implementation dependent..)

Macros should be provided for every peripheral separately if multiple devices are available - ie adc0, adc1 etc.. 

Typical direct usage
--------------------

```
adc0_init_default();
printf("ADC value: %d\n", adc0_read_immediate(channel_id));
```

For other options it is best to check out implementation specific file in arch/<manufacturer>/<cpu>/adc.h

General purpose parallel interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/gpio.h
* arch/gpio.h

A general purpose io is any kind of parallel device where it is possible to address each bit separately and which can be one or more bytes wide. It must be possible to write individual bits separately and also to write full "words" which are as wide as the width of the gpio interface. For internal devices it may be convenient to group all gpio interfaces together so that the user can use a continuous pin indexing scheme (ie for 3x8 bit on board parallel ports the pins can be numbered 0-24 in order to provide a generic way to access the port). 

Public interface
----------------

| parallel_interface | Description |
|--------------------|-------------|
void 		(*write_pin)(struct parallel_interface *self, uint16_t pin_number, uint8_t value) | sets pin of the output port either high or low. 
uint8_t (*read_pin)(struct parallel_interface *self, uint16_t pin_number) | used to read an input pin. Always returns 0 when reading an output pin. 
uint8_t	(*configure_pin)(struct parallel_interface *self, uint16_t pin_number, uint16_t flags) | configure pin to be input, output, pulled up, pulled down etc. Not all options may be supported by implementation. Function returns 0 on success and 1 on failure. 
uint8_t (*get_pin_status)(struct parallel_interface *self, timestamp_t *t_up, timestamp_t *t_down) | used to get status of the pin. Interface allows the implementation to track pin changes in the background and report values to the user of the interface. Returns pin status (went high/low etc.) and writes time in ticks for when pin went high, or went low. You should use time_ticks_to_us() to get interval in us. ticks is a value used for tracking cpu ticks. It only denotes a specific point in time. It is currently easier to keep track of time of the pin changes in the implementation rather than leaving this task to the user. This function is very useful for writing libraries that need to measure pulse intervals. The values of t_up and t_down must be updated by the implementation as way of measuring when the pin went high and when it goes low. Default value returned must be 0. 
uint8_t (*write_word)(struct parallel_interface *self, uint16_t addr, uint32_t value) | used to write byte or int to an io address. If you have pins PA0, PA1 .. PA7 and your registers are 8 bit long then writing to addr 0 should write all of PA pins at the same time. For implementations with larger registers, more bits may be written. This method is used to write multiple bits in one operation. 
uint8_t (*read_word)(struct parallel_interface *self, uint16_t addr, uint32_t *output) | user to read word from an io register. The size of the word depends on the implementation. It may be 8 bit or 16 bit or 32 bit. The size is equivalent to the full size of io registers of implementation. addr is the index of the io register. Implementation must check this value for a valid range and return error if it is invalid.  

Implementation macro guidelines
----------------------

| gpio method | Description |
|-------------|-------------|
gpio_init() | initializes the hardware gpio peripherals
gpio_configure(pin, fun) | provides means to configure pins as input, output, pullup etc. Although not as aux functions, that is the job of the specific aux peripheral driver to set up pins into aux state (ie to connect them to that peripheral) 
gpio_write_word(addr, value) | writes all bits at once on a port specified by addr index. 
gpio_read_word(addr, value) | reads all bits at once from port. 
gpio_write_pin(pin, val) | writes 1 or 0 to a specific pin. GPIO driver should provide all pin definitions as well so that user can easily access pins on the device. 
gpio_read_pin(pin) | reads gpio pin
gpio_clear(pin) | clears a pin (some platforms can do this as one atomic operation!)
gpio_set(pin) | sets a pin
gpio_enable_pcint(pin) | provides means to enable pin change interrupt on a pin. In most cases implementation should provide actual interrupt implementaiton and cache data for the user.
gpio_disable_pcint(pin) | disable pin change interrupt for a pin. 
uint8_t gpio_get_status(gpio_pin_t pin, timestamp_t *ch_up, timestamp_t *ch_down) | if implementation provides means to measure pulse widths of pulses inputted into the pin then this method provides means to read this data. The return value should indicate whether last event was pin going high or going low. 

Example usage
--------------------

```
	gpio_init();
		while(1){
			gpio_write_pin(GPIO_PB0, 1);
			delay_ms(500);
			gpio_write_pin(GPIO_PB0, 0);
			delay_ms(500);
		}
	}
```

I2C / two wire interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/twi.h
* arch/twi.h

Public interface
----------------

The public interface used by the i2c bus is packet_interface. This is because i2c is a protocol based state machine that expects some standard data to operate. In this respect it operates just like any packet interface - it sends data buffers and receives data into a buffer. One special case that takes the point home is this: i2c devices sometimes expect a write operation followed by a repeated start signal and then a read operation. This sequence is used to read values from an i2c device. Using packet interface allows us to model this behaviour since it has begin(), end() write() and read() methods. Serial interface would be inappropriate here.


| Interface method | Corresponding twi action |
|------------------|--------------------------|
void 					(*begin)(struct packet_interface *self) | must be called every time you want to start sending data. This call allows the driver to power on the device and prepare it for accepting new data.
void 					(*end)(struct packet_interface *self) | must be called when you do not intend to send more data for a while. This method signals the driver to release the bus to other devices since packet interfaces usually share underlying medium. It is also a good place to tell the other side that we are done transmitting data. 
uint32_t			(*write)(struct packet_interface *self, const uint8_t *data, uint16_t max_sz) | writes a packet to the device the max_sz parameter tells the device how big the packet is. The function can return before the data is actually sent. In that case the device will be busy and will not accept any data until the previous data has been sent
uint32_t			(*read)(struct packet_interface *self, uint8_t *data, uint16_t max_sz) | reads a packet from the device. May return before the operation completes! max_sz denotes maximum number of bytes that are read. If a packet that arrives in hardware is larger than max_sz then the function reads in max_sz bytes and returns an error in top 16 bits of the return value. The bottom 16 bits are number of bytes that have been read. You must always check status to see if previous operation has been completed before you make any changes to passed parameters because the method accesses the data directly and if you for example exit from a function where data is a stack variable without making sure that the operation has completed, the result may be a crash because the driver is not mandated to block until the data has been received. 
void					(*sync)(struct packet_interface *self) | instructs the hardware to send all the data it has in it's buffers and waits for the hardware to complete as well as makes sure that all pending operations have completed before exiting. 
uint16_t 			(*packets_available)(struct packet_interface *self) | returns the number of packets that are waiting to be read using getn. If a read operation is already in progress, this call should return one less packets or zero in the case where only one packet can be received at a time. 


Implementation macro guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
twi0_init(speed) | initializes twi peripheral to run at given clock speed
twi0_begin() | must be called when user is about to send a packet. 
twi0_end(void) | must be called when you do not intend to use the peripheral any more. It also waits for all previous operations to finish before returning so it can be used as way to sync with the peripheral. 
twi0_start_write(uint8_t *data, uint8_t data_sz) | starts a write operation on the bus. This method may return directly but the operation may be done in the background. The buffer that is supplied is used directly in order to save memory so it must not be destroyed! 
twi0_start_read(uint8_t *data, uint8_t data_sz) | starts a read operation and works much like the start_read() method. 
twi0_busy(void) | checks if twi is busy processing a transaction
twi0_success(void) | checks whether last operation completed successfully

Example usage
--------------------

```
char buffer[] = {DEVICE_ADDRESS, 'H', 'e', 'l', 'l', 'o'}; 
twi0_init(100000);
twi0_begin();
twi0_start_write(buffer, 6);
twi0_start_read(buffer, 6); // will read data into buffer starting at buffer[1].. 
twi0_end()
```

PWM peripheral interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/pwm.h
* arch/pwm.h

Public interface
----------------

PWM interface is for controlling pwm hardware. Usually it would be built in, timer driven pwm channels, but the system does not limit you to just on board pwm channel You could just as easily have a driver for an i2c or spi pwm controller export this kind of interface and this would allow you to pass it to any component that requires a pwm interface (such as for example a motor speed controller). 

The basic pulse width modulator interface consists of following methods: 

| Interface method | Corresponding twi action |
|------------------|--------------------------|
uint16_t 		(*set_channel_duty)(struct pwm_interface *self, uint8_t channel, uint16_t value_us) | Sets channel "on" time to value specified in microseconds. A pwm controller can have several channels. Channel id is implementation specific id of the channel. returns actual duty cycle time that has been set. 
uint16_t 		(*set_channel_period)(struct pwm_interface *self, uint8_t channel, uint16_t value_us) | Occasionally it may be beneficial to change period time of the pwm signal. Implementation should pick the prescaler value that gives the closest match to the specified period. This does not guarantee that the period value will be exact. but implementation should try to make it as exact as possible. returns actual period that has been set. 


Implementation macro guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
pwm0_enable() | enable pwm channel and configure it to settings most commonly seen in hobby controller context: ie pulse width between 0 and 2000 usec. The frequency is also kept as high as possible. Some servos may not be able to handle higher update rate but this can be fixed later if it becomes an issue. 
pwm0_set(speed) | sets pulse width in microseconds. 

Example usage
--------------------

pwm0_init();
pwm1_init();

pwm0_set(1500); // half throttle
pwm0_set(1000); // zero throttle
pwm0_set(2000); // full throttle

SPI peripheral interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/spi.h
* arch/spi.h

Public interface
----------------

SPI peripherals can be accessed by using standard serial_interface. A method for each peripheral must be provided: spi0_get_serial_interface() that returns struct serial_interface.


Implementation macro guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
hwspi0_set_clock(spi_rate) | sets spi clock rate. Implementation must provide appropriate devinces along the form of SPI_CLOCK_DIVXXX. 
hwspi0_set_mode(spi_mode) | sets spi mode (whether it's sampled on falling or leading edge of the clock. Valid modes should be SPI_MODE0, SPI_MODE1, SPI_MODE2 and SPI_MODE3. 
hwspi0_master() | configure this interface as spi master
hwspi0_slave() | configure this interface as spi slave
hwspi0_order_lsb_first() | sets order to lsb transmitted first (bit order)
hwspi0_order_msb_first() | sets order with msb first
hwspi0_interrupt_enable() | enables spi interrupt
hwspi0_interrupt_disable() | disables spi interrupt
hwspi0_enable() | enables spi interface
hwspi0_disable() | disables spi interface
hwspi0_config_gpio() | configures gpio pins for this spi interface (ie input/outputs etc). 
hwspi0_wait_for_transmit_complete() | waits for transmission to finish
hwspi0_error_collision() | checks collision flag (set if data is written before previous data has been sent)
hwspi0_init_default() | initializes spi peripheral with default settings. Usually SPI_MODE0, interrupt disabled, msb first, master, spi enabled. Clock is set to 1mhz (or something close, depending on used hardware) 
hwspi0_putc(ch) | writes a character to spi, does not 
hwspi0_getc(ch) | reads previously received character. since spi transmits and receives at the same time, the character is usually cached by implementation and you can then read it using this method. 
hwspi0_transfer(ch) | combines write and read into one op. Returns character received. 


Example usage
--------------------

```
hwspi0_init_default();

uint8_t data = hwspi0_transfer('X'); 
```
Timers and tick counters
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/time.h
* arch/time.h

Intro
-----

It is often necessary for implementation to provide a timer/counter for counting cpu clock cycles because this is the only way to do fairly exact timing when interrupts are enabled. More complex chips usually have a dedicated timer for this and an instruction to read clock counter. But for architectures that do not have this functionality, one of the timers needs to be used.

Usage of clock counter
---------------

In the rest of the system clock counter value can be used in many ways - from simple delays to asynchronous events. If you disable this functionality then some drivers may stop working, but ideally they would not even compile. There is a special config field called CONFIG_TIMESTAMP_COUNTER that enables timestamping functionality and drivers that need this functionality should be configured in the corresponding KConfig file to depend on it being set. 

Public interface
----------------

Timer counter currently does not export interfaces. This may change when drivers would require an abstract timer object. For now, timers are mostly only used directly by drivers in the arch layer. While higher level drivers mostly only use timestamping to calculate delays and timeouts. 


Implementation guidelines
----------------------

| Hardware method | Description |
|-----------------|-------------|
timestamp_init() | sets up timestamping counter to start counting. This configures one of the hardware timers to run at a frequency that is preferably exactly 1Mhz. But it may be 2Mhz or 8Mhz.. the value is implementation specific and the timestamp value returned by this method is in ticks count. So you need to convert it to microseconds in order to see what it is in microseconds.
timestamp_now() | gets current timestamp value in ticks. The value is ticks from cpu start. but remember that the value will wrap around after about 70 minutes. So do not measure overly long delays using this counter. The standard use is to capture the counter, do something and then check whether certain amount of time has elapsed. Functions below help you do this. 
timestamp_ticks_to_us(ticks) | converts ticks value to microseconds. 
timestamp_us_to_ticks(us) | converts microseconds to equivalent number of ticks
timestamp_before(unknown, known) | checks if unknown is before known. This function is required to handle overflow of the ticks counter correctly. 
timestamp_after(a,b) | checks if a is after b, also overflow safe
timestamp_from_now_us(us) | calculates a timeout value a number of microseconds in the future
timestamp_expired(timeout) | checks if a previously set timeout has expired
timestamp_delay_us(timestamp_t usec) | equivalent to busy loop delay, but more exact when interrupts are enabled. 
delay_us(usec) | a generic method that can be pinted directly to timestamp_delay_us
timestamp_ticks_since(timestamp) | returns ticks that have elapsed from some timestamp

There are more timer hardware specific methods but they will not be covered here for now. Instead, have a look at implementation specific time.h file to get an idea. 

Example usage
--------------------

```
timestamp_init();
timeout_t timeout = timeout_from_now(1000000); // 1s timeout
while(1){
	// do work
	if(timeout_expired(timeout))
		break; // will stop after 1 second
}
```

UART serial interface
----------------------------
Martin K. Schröder | info@fortmax.se

Files:

* arch/manufacturer/model/uart.h
* arch/uart.h

Intro
-----

As with many other peripherals, a chip can have several uarts. All uarts should export a serial_interface for generic access. 


Public interface
----------------

Check out definition of struct serial_interface in arch/interface.h 

Implementation guidelines
----------------------

Implementation is as always free to either use buffered interrupt driven uart or simple non buffered uart. The advantage of implementing a buffered uart is that all write operations can return to higher level code much quicker so that application can carry on while uart sends the data in the background. So it is always preferable to at least have a small buffer. 

| Hardware method | Description |
|-----------------|-------------|
 | 

Example usage
--------------------

It is up to higher level code to provide printf functionality. However, uart code may export a file descriptor for the uart making it possible to use standard fprintf() functionality pretty easily. If this is done, then global uart file descriptors are exported and named uartX_fd for each available uart. This removes the need to write custom printf functions because we can just use libc. 

```
uart0_init(32400);

// accessing directly
uart0_putc('X');

// using an interface
struct serial_interface serial = uart0_get_serial_interface();
char buffer[] = "Hello World!"; 
serial.putn(&serial, buffer, strlen(buffer));

// using file descriptor
fprintf(uart0_fd, "Hello %s\n", "World!"); 
```
 
 
 

Filesystem support
=====================



Currently supported filesystems
------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
 FAT  |  Fat filesystem   |  not ported 

 
Display Drivers
===============

Currently supported displays
---------------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| ILI9340  |  TFT display   |  yes  |
| LCDPFF8574   |   PCF8574 based lcd displays    |  yes  |
| LEDMATRIX | Led matrix display | not tested |
| SEVSEG | seven segment display | not tested | 
| SSD1306 | monochrome OLED display | yes | 

Human interface devices
==================

Currently supported input devices
-----------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| WIINUNCHUCK  |  Wii nunchuck   |  not tested  |

External IO peripherals
======================

Currently supported devices
---------------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
| L74HC165  |  Shift in register   |  yes  |
| L74HC4051   |   Multiplexer    |  yes  |
| L74HC595 | Shift out register | yes |
| PCF8574 | I2C GPIO expander | yes | 

Ethernet adapters
=================

Currently supported devices
-------------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
 ENC28J60  |  Ethernet driver IC   |  not tested  
 TCPIP   | Minimal IP stack    |  not tested  
 RFNET | Encrypted radio com | yes  

 

Radio devices
=============

Currently supported devices
--------------------

|  Name    |    What       |Status |
|----------|:-------------:|------:|
 NRF24L01  |  Radio module  |  not tested  

 
 
TTY Emulation Support
===================

Author: Martin K. Schröder, 2015

Currently supported tty interfaces
------------------------

TTY drivers need to use generic display interface so that it can be used with any GLCD display. 

|  Name    |    What       |Status |
|----------|:-------------:|------:|
 VT100  |  VT100 interface for a text screen   |  yes  

 
 
