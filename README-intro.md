Introduction
============
Author: Martin K. Schröder <mkschreder.uk@gmail.com>

This project originally started as collection of drivers for verious devices
that are usable with arduino. Then it transformed into a research project into
practical use of Adam Dunkel's protothreads for writing device drivers for a
small operating system. And finally, it has transfoming into a build system for
building firmware images for various microcontrollers.  

It now supports realtime preemptive scheduling with ability to for example run
multiple context switched threads on an arduino. The design has seen a lot of
changes during the protothreads research phase, but is slowly stabilizing
towards using protothreads only for stateful user processes (device drivers
turned out to be a bad candidate for protothreads - however the concept is very
nice for modelling tiny local threads within a single process). 

Note: this readme really needs to be rewritten because a lot of things have
changed. For now I'll leave it like this until I find time to rewrite it. 

LibK is a lightweight firmware development library that is designed for flash
based, resource constrained microcontrollers that are not able to run a full
modern operating system. LibK uses protothread-based, cooperative multitasking
to make all I/O operatins non-blocking and to make the most of the already
constrained CPU by never wasting any clock cycles waiting for an I/O operation
to complete. 

Included in LibK you will find many device drivers that make it possible to use
the supported devices in your project directly without having to implement the
device drivers yourself. The device drivers have also been rewritten by me to
use cooperative multitasking so that no device can ever slow down your
application just because it is slow. 

Using LibK you can build portable applications for AVR-8, STM32 and PC. 


Design goals
------------

- To maintain small memory footprint and use minimal amount of ram
- To take advantage of as much of the (limited) CPU power as possible 
- To maintain high portability across all of the supported platforms
- To minimize the amount of code inside the application that needs to be
  changed to support a new platform. 

How the design goals are currently implemented: 

- LibK places as much constant data as possible into flash and thus frees up a
  lot of ram for use in the application
- LibK uses cooperative multitasking inside the kernel and all drivers are
  written to never block the cpu while waiting for an external event or I/O. 
- LibK uses asynchronous I/O at all times, meaning that no read or write
  operation is ever blocking. This allows for other tasks to happen while a
  task is waiting for io. 
- LibK uses protothreads instead of conventional threads - this means that
  threads never have any stack and all threads execute inside the same stack
  space of the main application. It also have many other advantages that are
  discussed further in this document. 

Building libk on Linux
----------------------

Note: LibK build system is originally designed for building on Ubuntu linux so
it may require modifications if you want to build on a different system. The
build process uses Makefiles that it includes from all of the subfolders. Each
subfolder has its own Makefile, KConfig file and a README.md (docs) file. All
of these files are aggregated by make and then compiled into their respective
targets. 

To cross compile LibK you will also need appropriate gcc toolchain for your
hardware target. 

* avr-gcc and avr-libc for compiling for atmega, arduino, avr32
* arm-none-eabi-gcc for compiling for sam3, stm32 and other arm architectures
* pandoc for compiling this PDF from all readme files (although you should have this PDF included)
* clang (scan-build) for enabling static analysis of the code during the build
* lib-curses for compiling menuconfig (for editing libk configurations)
* avrdude for flashing avr chips

On Ubuntu: 

	sudo apt-get install gcc-avr gcc-arm-none-eabi avr-libc pandoc libncurses5-dev avrdude

Configure: 

	make menuconfig

Build:

	make
	
Flashing firmware on your hardware
--------------------------

Supported architectures
------------------

| Manufacturer | Chip | status |
|--------------|------|--------|
| Atmel | ATMega328p | my primary focus right now |
| Atmel | AT91SAM3 ARM | my secondary focus but now yet fully supported |
| ST | STM32F103 | peripheral library is included, but device interfaces not implemented yet |

Device driver support
---------------------

Device drivers in libk operate on a higher level than architecture code. So
they are largely architecture agnostic. Device drivers typically use interfaces
to access services provided by the architecture and they can also export
interfaces to other drivers in order to eliminate dependencies between drivers
on each other.

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

Drivers are tested on avr. Other architectures may not have full drivers
support until there is stable architecture abstraction layer written for that
architecture. 

(devices marked as quarantine are drivers that have not been updated yet after
changes to the core api. Code has been included in the source tree but it has
not yet been updated to work nicely with other facilities of libk)


