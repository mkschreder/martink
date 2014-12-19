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
