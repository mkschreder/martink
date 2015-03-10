#pragma once

struct pio_if;
typedef struct pio_if **pio_dev_t;

/**
 * GPIO interface is for implementing any type of bit wise parallel io. A lot 
 * of device can implement this kind of interface - i2c parallel io chips, shift 
 * registers that connect over spi, soc gpio pins, etc. Characteristic of this 
 * interface is that it consists of arbitrary number of "pins" that can be written
 * and read, as well as configured to have pullups.
 *
 * The parallel interface is designed specifically for manipulating IO lines in a
 * bit-wise fashion. It is therefore not designed for direct byte wise or word wise write
 * or read operations. 
 **/
struct pio_if {
	/// set pin of the output port either high or low. 
	void 		(*write_pin)(pio_dev_t self, uint16_t pin_number, uint8_t value);
	/// used to read an input pin. Always returns 0 when reading an output pin. 
	uint8_t (*read_pin)(pio_dev_t self, uint16_t pin_number);
	/// configure pin to be input, output, pulled up, pulled down etc. Not all options may
	/// be supported by implementation. Function returns 0 on success and 1 on failure. 
	uint8_t	(*configure_pin)(pio_dev_t self, uint16_t pin_number, uint16_t flags);
	/// used to get status of the pin. Interface allows the implementation to track pin
	/// changes in the background and report values to the user of the interface.
	/// Returns pin status (went high/low etc.) and writes time in ticks for when pin
	/// went high, or went low. You should use time_ticks_to_us() to get interval in us.
	/// ticks is a value used for tracking cpu ticks. It only denotes a specific point in
	/// time. It is currently easier to keep track of time of the pin changes in the
	/// implementation rather than leaving this task to the user. This function is very
	/// useful for writing libraries that need to measure pulse intervals. The values
	/// of t_up and t_down must be updated by the implementation as way of measuring when
	/// the pin went high and when it goes low. Default value returned must be 0. 
	//uint8_t (*get_pin_status)(pio_dev_t self, uint16_t pin_number, timestamp_t *t_up, timestamp_t *t_down);
	/// used to write byte or int to an io address. If you have pins PA0, PA1 .. PA7 and
	/// your registers are 8 bit long then writing to addr 0 should write all of PA pins
	/// at the same time. For implementations with larger registers, more bits may be
	/// written. This method is used to write multiple bits in one operation. 
	uint8_t (*write_word)(pio_dev_t self, uint16_t addr, uint32_t value);
	/// user to read word from an io register. The size of the word depends on the
	/// implementation. It may be 8 bit or 16 bit or 32 bit. The size is equivalent to
	/// the full size of io registers of implementation. addr is the index of the io
	/// register. Implementation must check this value for a valid range and return
	/// error if it is invalid. 
	uint8_t (*read_word)(pio_dev_t self, uint16_t addr, uint32_t *output); 
}; 

/// set pin of the output port either high or low. 
#define pio_write_pin(dev, pin, value) (*dev)->write_pin(dev, pin, value)
//static inline void 		pio_write_pin(pio_dev_t self, uint16_t pin_number, uint8_t value){
//	(*self)->write_pin(self, pin_number, value);
//}

/// used to read an input pin. Always returns 0 when reading an output pin. 
#define pio_read_pin(dev, pin) (*dev)->read_pin(dev, pin)
//static inline uint8_t pio_read_pin(pio_dev_t self, uint16_t pin_number){
//	return (*self)->read_pin(self, pin_number);
//}

/// configure pin to be input, output, pulled up, pulled down etc. Not all options may
/// be supported by implementation. Function returns 0 on success and 1 on failure. 
#define pio_configure_pin(dev, pin, flags) (*dev)->configure_pin(dev, pin, flags)
//static inline uint8_t	pio_configure_pin(pio_dev_t self, uint16_t pin_number, uint16_t flags){
//	return (*self)->configure_pin(self, pin_number, flags);
//}

/// used to write byte or int to an io address. If you have pins PA0, PA1 .. PA7 and
/// your registers are 8 bit long then writing to addr 0 should write all of PA pins
/// at the same time. For implementations with larger registers, more bits may be
/// written. This method is used to write multiple bits in one operation. 
#define pio_write_word(dev, addr, value) (*dev)->write_word(dev, addr, value)
//static inline uint8_t pio_write_word(pio_dev_t self, uint16_t addr, uint32_t value){
//	return (*self)->write_word(self, addr, value);
//}

/// user to read word from an io register. The size of the word depends on the
/// implementation. It may be 8 bit or 16 bit or 32 bit. The size is equivalent to
/// the full size of io registers of implementation. addr is the index of the io
/// register. Implementation must check this value for a valid range and return
/// error if it is invalid. 
#define pio_read_word(dev, addr, out) (*dev)->read_word(dev, addr, out)
//static inline uint8_t pio_read_word(pio_dev_t self, uint16_t addr, uint32_t *output){
//	return (*self)->read_word(self, addr, output);
//}
