#include "interface.h"


/// begins a serial transactions. A device driver can for example clear 
/// any cache buffers when begin is called. This call may block until device is ready. 
int16_t serial_begin(serial_dev_t dev){
	return (*dev)->begin(dev); 
}

/// finishes a transaction. This call may block until all data has been
/// transfered by the device. It serves as a way to sync transaction with
/// other actions necessary to control a device. 
int16_t serial_end(serial_dev_t dev){
	return (*dev)->end(dev); 
}

/// writes one character to a generic serial interface
uint16_t serial_putc(serial_dev_t dev, uint8_t ch){
	return (*dev)->put(dev, ch); 
}

/// reads one character form a generic serial interface
uint16_t serial_getc(serial_dev_t dev){
	return (*dev)->get(dev);
}

/// writes a string of characters to a generic serial interface
size_t	  serial_putn(serial_dev_t dev,
	const uint8_t *data, size_t max_sz){
	return (*dev)->putn(dev, data, max_sz);
}

/// reads a string of characters from a generic serial interface
size_t serial_getn(serial_dev_t dev,
	uint8_t *data, size_t max_sz){
	return (*dev)->getn(dev, data, max_sz); 
}

/// gets number of bytes waiting in rx buffer
size_t  serial_waiting(serial_dev_t dev){
	return (*dev)->waiting(dev);
}

/****************************
* PIO 
*****************************/

/// set pin of the output port either high or low. 
void 		pio_write_pin(pio_dev_t self, uint16_t pin_number, uint8_t value){
	(*self)->write_pin(self, pin_number, value);
}

/// used to read an input pin. Always returns 0 when reading an output pin. 
uint8_t pio_read_pin(pio_dev_t self, uint16_t pin_number){
	return (*self)->read_pin(self, pin_number);
}

/// configure pin to be input, output, pulled up, pulled down etc. Not all options may
/// be supported by implementation. Function returns 0 on success and 1 on failure. 
uint8_t	pio_configure_pin(pio_dev_t self, uint16_t pin_number, uint16_t flags){
	return (*self)->configure_pin(self, pin_number, flags);
}

/// used to get status of the pin. 
//uint8_t pio_get_pin_status(pio_dev_t self, uint16_t pin_number, timestamp_t *t_up, timestamp_t *t_down){
//	return (*self)->get_pin_status(self, pin_number, t_up, t_down);
//}

/// used to write byte or int to an io address. If you have pins PA0, PA1 .. PA7 and
/// your registers are 8 bit long then writing to addr 0 should write all of PA pins
/// at the same time. For implementations with larger registers, more bits may be
/// written. This method is used to write multiple bits in one operation. 
uint8_t pio_write_word(pio_dev_t self, uint16_t addr, uint32_t value){
	return (*self)->write_word(self, addr, value);
}

/// user to read word from an io register. The size of the word depends on the
/// implementation. It may be 8 bit or 16 bit or 32 bit. The size is equivalent to
/// the full size of io registers of implementation. addr is the index of the io
/// register. Implementation must check this value for a valid range and return
/// error if it is invalid. 
uint8_t pio_read_word(pio_dev_t self, uint16_t addr, uint32_t *output){
	return (*self)->read_word(self, addr, output);
}

