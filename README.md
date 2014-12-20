MartinK IC Firmware Project (LibK)
===============

LibK is more than just a firmware library. It is a toolchain, a philosophy, a way to organize your code. It is maximum flexibility at minimum cost. It is a collection of reusable components for writing bare metal embedded applications.

For documentation check out the following pages:

* [Detailed hardware and api documentation for libk](https://github.com/mkschreder/martink/tree/master/doc/)

It is designed for microcontrollers, flash based system on chip devices, small controllers with under 10kb of flash, controllers with as little as 2kb of ram, arm chips and avr chips. For many small devices linux simply does not work. What you wish you had is a library that makes it easy for you to write reusable code and abstracts away the absolutely lowest levels of hardware interaction with minimal overhead. 

Do you ever have problems with figuring out how to use 'UDSR' or 'UCR' or 'BCC' or any other obscure register just to get some piece of hardware configured correctly? Do you ever always have datasheet open for the cpu you are working on just because you can't remember which bits in what registers you need to set?

Yes unfortunately there has not been a good alternative to linux speciffically designed for tiny microcontrollers. This has resulted in many people designing their own solutions that are often completely incompatible when you find code from somebody else for another on chip peripheral. 

LibK provides a framework for solving all of these problems. It comes with specialized build system based on menuconfig that provides a highly fine grained build configuration and lets you only compile the things that you need. It also makes it easy to switch between different targets that require completely different low level implementations.

Building
--------------

To build with LibK you need to make sure that you have necessary toolchain installed for your architecture. For avr it's avr-gcc, for arm it's either arm-none-eabi-* or arm-linux-eabi-* packages. You also need libncurses-dev (for menuconfig). 

You can then start build process like this: 

    make menuconfig
    make

If everything goes well, the file that will be built will be libk.a. This is a library that you will then link your application with. It is often easier to integrate the build process for libk into the build process for your project. It is also good to make a symbolik link to libk code directory directly inside your project directory. This way you don't have to keep track of multiple copies of libk.

For an example on how to integrate the two build processes using make, you can check out my quadcopter controller project based on libk here: 
Example project: https://github.com/mkschreder/bettercopter

Supported architectures
------------------

| Manufacturer | Chip | status |
|==============|======|========|
| Atmel | ATMega328p | my primary focus right now |
| Atmel | AT91SAM3 ARM | my secondary focus but now yet fully supported |
| ST | STM32F103 | peripheral library is included, but device interfaces not implemented yet |

Device driver support
---------------------

Device drivers in libk operate on a higher level than architecture code. So they are largely architecture agnostic. Device drivers typically use interfaces to access services provided by the architecture and they can also export interfaces to other drivers in order to eliminate dependencies between drivers on each other.

| Device class | Device model | Support | Interfaces used | Interfaces exported | 
| Board | Multiwii V2.5 | Supported | `flight_controller` interface | |
| Board | Arduino Pro Mini | Supported | Architecture ATMega328p | |
| Board | Stm32f103 development board | planned | | |
| Board | Crius ATmega256 flight control | planned | |  |
| Crypto | AES 256 | Supported |  | |
| Display | ILI9340 | Supported | parallel_interface and serial_interface | |
| Display | Parallel LCD | Supported | parallel_interface | |
| Display | 7 segment led | Supported | parallel_interface | |
| Display | 8x8 Led matrix | Supported | parallel_interface | |
| Display | ssd1306 OLED | Supported | packet_interface (i2c) | |
| Filesystem |  | Planned | | |
| HID | WiiNunchuck | Supported | packet_interface (i2c) | |
| IO | 74HC165 | Supported | serial_interface, parallel_interface | |
| IO | 74HC4051 | Supported | parallel_interface | |
| IO | 74HC595 | Supported | serial_interface, parallel_interface | |
| IO | PCF8574 | Supported | packet_interface (i2c) | planned: parallel_interface |
| NET | ENC28J60 Supported | serial_interface | planned: packet_interface |
| NET | TCPIP | Supported | packet_interface | planned: serial_interface |
| RADIO | NRF24L01 | Supported | serial_interface | planned: packet_interface |
| SENSOR | ACS712 | (quarantine) | analog_interface | |
| SENSOR | ADXL345 | (quarantine) | packet_interface (i2c) | |
| SENSOR | AMT345 | (quarantine) | analog_interface | |
| SENSOR | BH1750 | (quarantine) | packet_interface (i2c) | |
| SENSOR | BMP085 | Supported | packet_interface (i2c) | |
| SENSOR | DHT11 | (quarantine) | parallel_interface | |
| SENSOR | DS18B20 | (quarantine) | parallel_interface | |
| SENSOR | FS300A | (quarantine) | | |
| SENSOR | HCSR04 | (quarantine) | parallel_interface | |
| SENSOR | HMC5883L | Supported | packet_interface (i2c) | |
| SENSOR | L3G4200D | (quarantine) | packet_interface (i2c) | |
| SENSOR | LDR | (quarantine) | analog_interface | |
| SENSOR | MMA7455 | (quarantine) | packet_interface (i2c) | |
| SENSOR | MPU6050 | Supported | packet_interface (i2c)| |
| SENSOR | NTCTEMP | (quarantine) | analog_interface | |
| SENSOR | TSL235 | (quarantine) | | |
| TERMINAL | VT100 | Supported | framebuffer_interface | serial_interface |

(devices marked as quarantine are drivers that have not been updated yet after changes to the core api. Code has been included in the source tree but it has not yet been updated to work nicely with other facilities of libk)

Design goals
------------

- To make it a portable and highly configurable alternative to linux but for microcontrollers
- To provide a single place where many smaller driver projects are gathered and thus make these drivers also portable across many different SoCs. 
- To keep in mind that the final code may have to run on a system with just 2k of ram. This is important because many design decisions and code practices are used throughout the firmware code to keep the amount of data stored in memory to an absolute minimum. 
- To support compilation for desktop as well as microcontroller to enable efficient testing on desktop before running the code on a microcontroller. 

Architechture
--------------

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
