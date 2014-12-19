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

