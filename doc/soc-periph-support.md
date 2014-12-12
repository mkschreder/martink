SOC support
==========

Directory arch/ contains soc peripheral interface that aims to be generic yet as lightweight as possible. This layer enables all other dirvers to be written in a portable fashion so that we can reuse code across multiple socs. 

Device interfaces belonging to this layer include: 

- UART interfaces (0-4)
- SPI interfaces (0-4)
- I2C 
- CAN bus
- GPIO
- PWM (hardware)
- TIMING
- ADC
- DAC

The purpose of this layer is not to have a fine grained interface - there are vendor specific libraries for that. But rather to provide a set of common services that higher levels can rely upon. 

UART
-----

Interface: 
- uartX_init(baud): initialize uart X and configure it's speed in bps. Enables any needed clocks, sets gpio modes to chip specific values and enables tx and rx. 
- uartX_putc(ch): write single character to UART X
- uartX_getc(ch): read single character and status from uart X. Should return at least uint16_t in order to be able to tell if there was no data available (most signifacant byte is status). 
- uartX_write(buf, size): write buffer to uart
- uartX_read(buf, max_size): read bytes from uart. Should be non blocking and return number of bytes read. 

Utility functions: 
- uartX_printf(fmt, ...): printf
- uartX_puts(str): write string to uart

Note: utility functions perhaps should be implemented supporting any kind of stream interface, not just uarts. This is subject for thought. 

SPI 
---

Interface: 
- spiX_init(): initialize spi interface 
- spiX_writereadbyte(ch): write ch and return read byte

I2C/TWI interface
------------------

Interface: 
- twiX_init(): init twi interface 
