 

/*
 * A serial interface is any device that is capable of sending and receiving
 * data over some kind of channel. A serial interface is typically a stream
 * it is not a transaction based interface, but rather a stream based one. 
 * Data can usually be transfered in both directions and we do not use any 
 * kind of addressing scheme. A serial interface is a point to point channel
 * between two devices. 
 * 
 * Each of the interface methods takes the interface structure itself as first arg.
 * This pointer can later be converted to device specific context pointer using
 * container_of() macro (currently defined in util.h)
 */
#pragma once


struct serial_if;
typedef struct serial_if **serial_dev_t;

struct serial_if {
	/// getc reads one character from the interface
	/// returns UART_NO_DATA if none available
	uint16_t 			(*get)(serial_dev_t self); 
	/// putc writes a character into the interface
	/// waits for a timeout of 1-2 bytes if output buffer is full 
	/// discards data if tx buffer is full and no data can be sent
	/// Returns status of transaction. must return reasonably immidiately. 
	uint16_t 			(*put)(serial_dev_t self, uint8_t ch); 
	/// putn writes a buffer into the interface
	/// returns number of bytes written
	/// should return only when data has been at least copied into internal buffer of the driver
	/// is not required to wait until data has been transmitted on wire
	size_t				(*putn)(serial_dev_t self, const uint8_t *data, size_t max_sz);
	/// reads a number of characters from device
	/// returns number of bytes read
	/// returns always without waiting for more data to arrive
	/// always works with internal rx buffer of the interface
	size_t				(*getn)(serial_dev_t self, uint8_t *data, size_t max_sz);
	/// begins a new transaction
	int16_t				(*begin)(serial_dev_t self); 
	/// ends current transaction
	int16_t				(*end)(serial_dev_t self);
	/// returns current number of bytes available for reading
	size_t 				(*waiting)(serial_dev_t self);
}; 

/// begins a serial transactions. A device driver can for example clear 
/// any cache buffers when begin is called. This call may block until device is ready. 
#define serial_begin(dev) (*dev)->begin(dev)
//static inline int16_t serial_begin(serial_dev_t dev){
//	return (*dev)->begin(dev); 
//}

/// finishes a transaction. This call may block until all data has been
/// transfered by the device. It serves as a way to sync transaction with
/// other actions necessary to control a device.
#define serial_end(dev) (*dev)->end(dev) 
//static inline int16_t serial_end(serial_dev_t dev){
//	return (*dev)->end(dev); 
//}

/// writes one character to a generic serial interface
#define serial_putc(dev, ch) (*dev)->put(dev, ch)
//static inline uint16_t serial_putc(serial_dev_t dev, uint8_t ch){
//	return (*dev)->put(dev, ch); 
//}

/// reads one character form a generic serial interface
#define serial_getc(dev) (*dev)->get(dev)
//static inline uint16_t serial_getc(serial_dev_t dev){
//	return (*dev)->get(dev);
//}

/// writes a string of characters to a generic serial interface
#define serial_putn(dev, data, size) (*dev)->putn(dev, data, size)
//static inline size_t	  serial_putn(serial_dev_t dev,
//	const uint8_t *data, size_t max_sz){
//	return (*dev)->putn(dev, data, max_sz);
//}

/// reads a string of characters from a generic serial interface
#define serial_getn(dev, data, size) (*dev)->getn(dev, data, size)
//static inline size_t serial_getn(serial_dev_t dev,
//	uint8_t *data, size_t max_sz){
//	return (*dev)->getn(dev, data, max_sz); 
//}

/// gets number of bytes waiting in rx buffer
#define serial_waiting(dev) (*dev)->waiting(dev)
//static inline size_t  serial_waiting(serial_dev_t dev){
//	return (*dev)->waiting(dev);
//}

/// status codes returned by methods of the interface
#define SERIAL_PARITY_ERROR			0x1000
#define SERIAL_FRAME_ERROR      0x0800      
#define SERIAL_OVERRUN_ERROR    0x0400        
/// returned by putc if outgoing buffer is full
#define SERIAL_BUFFER_FULL  		0x0200             
/// returned by getc if no data is available
#define SERIAL_NO_DATA          0x0100
