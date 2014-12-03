MartinK IC Firmware Project
===============

MartinK firmware provides a highly configurable software layer between low level integrated circuit hardware and a higher level application. It is designed for making it easy to build projects such as quadcopters, sensor nodes, 3d printers etc. It is designed to be a versatile firmware library aimed at developers writing code for embedded microcontrollers. It is like linux, but designed for very very constrained systems. Many design goals implement this and enable the final application to be <10kb. 

Do you ever have problems with figuring out how to use 'UDR' or 'UCR' or 'BCC' register just to get something printed on the screen? Yes unfortunately there has not been a good alternative to linux speciffically designed for tiny microcontrollers. This resulted in many people designing their own solutions that are often completely incompatible when you find code from somebody else for another on chip peripheral. 

MartinK firmware provides a framework for solving all of these problems by using menuconfig just like linux kernel and by structuring the code such that many different peripherals can be supported while still maintaining excellent code readability and final product configurability. 

You compile this firmware just like you compile linux: 

    make menuconfig
    make

Supported hardware
===============

I'm hoping this list will grow but at the moment it is mainly populated by me. New drivers are included as I find working code or write a custom driver for some peripheral that I am currently working with. Thus the codebase may grow rather slowly. 

There are many files in this codebase that have been adopted from various other open source projects. Thus the license of the code is GPL, just like linux kernel. This is in fact a good thing, because if we want the project to grow, we must all contribute with code that others may not be able to write themselves. And we have to always keep the following design goals in mind:  

Design goals
===============

- To make it a portable and highly configurable alternative to linux but for microcontrollers
- To provide a single place where many smaller driver projects are gathered and thus make these drivers also portable across many different SoCs. 
- To keep in mind that the final code may have to run on a system with just 2k of ram. This is important because many design decisions and code practices are used throughout the firmware code to keep the amount of data stored in memory to an absolute minimum. 
- To support compilation for desktop as well as microcontroller to enable efficient testing on desktop before running the code on a microcontroller. 

Architechture
===============

The system uses makefiles for building the projects. This is a good thing because makefiles are extremely powerful and allow for very specialized compilation process. Through use of separate makefiles in different parts of the project we can specify exactly which files are compiled and included. We can also load configuration from menuconfig directly into a makefile and write rules based on the config. This build system is highly influenced by the very efficient build system used by linux kernel. 

We use mconf utility for generating the .config file which is included in makefiles and autoconf.h file which can be included into the application. Mconf (menuconfig) allows us to specify dependencies between different modules of the project and allow for highly specialized conditional compilation of different files for different systems. 

The final result produced by the system is a built-in.o file that contains all the code for the main kernel. This file is then linked into your project to allow it to access code that talks to different peripherals such as on chip UART or SPI or I2C or any other supported device. 

Many boards can be supported, by providing board speciffic files in the "boards" directory. Each file correspond to a supported board. Your application should do things like setting PWM output values by calling set_pin() method that is defined for every board. This method can do much more than just setting a GPIO pin. It can also take an integer value specifying PWM level and use it to fill appropriate hardware registers to generate output. Your board file should be speciffically designed for every board and it should provide an interface for the rest of the application for accessing these board peripherals. 

Many drivers for out of chip devices (such as I2C peripherals) are completely reusable on many different architectures. For this reason, we group all architecture speciffic files into arch/ directory. The purpose of the architecture speciffic files is to provide an interface for the DRIVERS which they can use to access standard peripherals such as I2C or SPI busses. The arch directory should therefore only contain files that implement ON CHIP peripherals that the IC uses to communicate with the outside world. 

When arch layer is ported to a different SoC, it should be possible to recompile the kernel and run all other drivers on that platform without any other changes. 

Known issues
===============

The way that devices are currently accessed is by using a prefix: ie uart_init() or spi_readwritebyte(). This may become an issue when we begin supporting boards with several uarts. This requires some thinking because it is not possible to use fancy things like dynamically allocated arrays of function pointers that would make every device completely interchangable. All devices in the kernel are sortof hardwired once it is compiled. This allows for very tiny memory footprint and allows the code to run well on small ICs with very little ram. Probably this can be partly solved by simply implementing uart1_x() groups of methods that would allow accesing the uart1 instead of uart0. This is how it has been done in many other microcontroller projects. Options can then be added into menuconfig to allow some higher level peripheral to be configured to use uart1 instead of uart0 and basically the problem may be solved. 

Another issue: drivers need to be revised and fixed so that there are no internal conflicts. Many drivers come from projects where it has been decided that the driver would have exclusive access to some peripheral. But when we have several devices, exclusive access is not possible. So one driver may set a register and another driver may accidentally change it. For the most part this has been fixed, but there are some drivers for components that I have not been able to test. So compile, test, if it works fine, if it doesn't, it should be fairly easy to fix. Then submit a patch so we can get it fixed as well.

Development environment
===============

For reasons that using makefiles and utilities that are otherwise standard on linux is hard on other systems, it is recommended that you use linux (or maybe macos) for working on this project. It is simply much easier to write a powerful build system like menuconfig on linux than it is on windows. For compilation use GCC, for scripting use bash, for editing use whatever editor you like. If you are on windows then simply run a version of Ubuntu in VirtualBox and map files from your host system into the emulated linux and build as usual while still using windows for other things. 
