/**
	Flexible hardware abstraction layer for socs

	martink firmware project is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	martink firmware is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with martink firmware.  If not, see <http://www.gnu.org/licenses/>.

	Author: Martin K. Schröder
	Email: info@fortmax.se
	Github: https://github.com/mkschreder
*/

#pragma once

#include <inttypes.h>
#include <stddef.h>
#include <sys/types.h>

#include <thread/pt.h>

#include "time.h"

/**
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
int16_t serial_begin(serial_dev_t dev);

/// finishes a transaction. This call may block until all data has been
/// transfered by the device. It serves as a way to sync transaction with
/// other actions necessary to control a device. 
int16_t serial_end(serial_dev_t dev);

/// writes one character to a generic serial interface
uint16_t serial_putc(serial_dev_t dev, uint8_t ch);

/// reads one character form a generic serial interface
uint16_t serial_getc(serial_dev_t dev);

/// writes a string of characters to a generic serial interface
size_t	  serial_putn(serial_dev_t dev,
	const uint8_t *data, size_t max_sz);
	
/// reads a string of characters from a generic serial interface
size_t serial_getn(serial_dev_t dev,
	uint8_t *data, size_t max_sz);
	
/// gets number of bytes waiting in rx buffer
size_t  serial_waiting(serial_dev_t dev);

/// status codes returned by methods of the interface
#define SERIAL_PARITY_ERROR			0x1000
#define SERIAL_FRAME_ERROR      0x0800      
#define SERIAL_OVERRUN_ERROR    0x0400        
/// returned by putc if outgoing buffer is full
#define SERIAL_BUFFER_FULL  		0x0200             
/// returned by getc if no data is available
#define SERIAL_NO_DATA          0x0100

/**
 * A packet interface is similar to a serial interface but instead operates on
 * chunks of data. Whereas a serial interface guarantees that you either get or
 * not get an individual byte of data, a packet interface does the same for
 * chunks, meaning that you either get or not get a packet - there is no possibility
 * to only get a partial packet.
 *
 * Packet interfaces are useful for message based communication schemes and bus
 * types that operates by means of "start -> data bytes -> stop" pattern. It
 * does not however handle addressing - this is the job of the underlying device
 * that this interface is used to access. All addressing and protocol data must
 * be passed inside the packet sent over this interface. This is another point
 * in which the packet device differs from a stream device. 
 */
/*
struct packet_interface {
	/// must be called every time you want to start sending data. This call allows the
	/// driver to power on the device and prepare it for accepting new data.
	void 					(*begin)(struct packet_interface *self);
	/// must be called when you do not intend to send more data for a while. This method
	/// signals the driver to release the bus to other devices since packet interfaces
	/// usually share underlying medium. It is also a good place to tell the other side
	/// that we are done transmitting data. 
	void 					(*end)(struct packet_interface *self); 
	/// putn writes a packet to the device
	/// the max_sz parameter tells the device how big the packet is. The function
	/// can return before the data is actually sent. In that case the device will
	/// be busy and will not accept any data until the previous data has been sent
	uint32_t			(*write)(struct packet_interface *self, const uint8_t *data, uint16_t max_sz);
	/// reads a packet from the device. May return before the operation completes! 
	/// max_sz denotes maximum number of bytes that are read. If a packet that arrives
	/// in hardware is larger than max_sz then the function reads in max_sz bytes and
	/// returns an error in top 16 bits of the return value. The bottom 16 bits are
	/// number of bytes that have been read. You must always check status to see
	/// if previous operation has been completed before you make any changes to
	/// passed parameters because the method accesses the data directly and if you
	/// for example exit from a function where data is a stack variable without making
	/// sure that the operation has completed, the result may be a crash because the
	/// driver is not mandated to block until the data has been received. 
	uint32_t			(*read)(struct packet_interface *self, uint8_t *data, uint16_t max_sz);

	/// instructs the hardware to send all the data it has in it's buffers and
	/// waits for the hardware to complete as well as makes sure that all pending
	/// operations have completed before exiting. 
	void					(*sync)(struct packet_interface *self);
	/// returns the number of packets that are waiting to be read using getn. If
	/// a read operation is already in progress, this call should return one less packets
	/// or zero in the case where only one packet can be received at a time. 
	uint16_t 			(*packets_available)(struct packet_interface *self);
};

#define p_begin(ptr_iface) ptr_iface->begin(ptr_iface)
#define p_end(ptr_iface) ptr_iface->end(ptr_iface);
#define p_write(ptr_iface, data, size) ptr_iface->write(ptr_iface, data, size)
#define p_read(ptr_iface, data, size) ptr_iface->read(ptr_iface, data, size)
#define p_sync(ptr_iface) ptr_iface->sync(ptr_iface)
*/
#define PK_ERR_INVALID 0x00010000L

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
void 		pio_write_pin(pio_dev_t self, uint16_t pin_number, uint8_t value);

/// used to read an input pin. Always returns 0 when reading an output pin. 
uint8_t pio_read_pin(pio_dev_t self, uint16_t pin_number);

/// configure pin to be input, output, pulled up, pulled down etc. Not all options may
/// be supported by implementation. Function returns 0 on success and 1 on failure. 
uint8_t	pio_configure_pin(pio_dev_t self, uint16_t pin_number, uint16_t flags);

/// used to get status of the pin. 
//uint8_t pio_get_pin_status(pio_dev_t self, uint16_t pin_number, timestamp_t *t_up, timestamp_t *t_down);

/// used to write byte or int to an io address. If you have pins PA0, PA1 .. PA7 and
/// your registers are 8 bit long then writing to addr 0 should write all of PA pins
/// at the same time. For implementations with larger registers, more bits may be
/// written. This method is used to write multiple bits in one operation. 
uint8_t pio_write_word(pio_dev_t self, uint16_t addr, uint32_t value);

/// user to read word from an io register. The size of the word depends on the
/// implementation. It may be 8 bit or 16 bit or 32 bit. The size is equivalent to
/// the full size of io registers of implementation. addr is the index of the io
/// register. Implementation must check this value for a valid range and return
/// error if it is invalid. 
uint8_t pio_read_word(pio_dev_t self, uint16_t addr, uint32_t *output);

/// ===========================================

struct i2c_interface;
typedef struct i2c_interface **i2c_dev_t;


/**
 * I2C device interface used for reading and writing i2c devices.
 */
struct i2c_interface {
	uint32_t			(*write)(i2c_dev_t self,
		uint8_t address, const uint8_t *data, uint16_t max_sz);

	uint32_t			(*read)(i2c_dev_t self,
		uint8_t address, uint8_t *data, uint16_t max_sz);

	/// returns -1 on fail and 1 on success
	int16_t 			(*stop)(i2c_dev_t self); 
	
	//void	(*wait)(i2c_dev_t self, uint8_t addr); 
	
	uint8_t (*status)(i2c_dev_t self, uint16_t flags); 
	uint8_t (*open)(i2c_dev_t self); 
	void 		(*close)(i2c_dev_t self); 
};

uint32_t i2c_write(i2c_dev_t dev,
	uint8_t address, const uint8_t *data, uint16_t max_sz);

uint32_t	i2c_read(i2c_dev_t dev,
	uint8_t address, uint8_t *data, uint16_t max_sz);

int16_t i2c_stop(i2c_dev_t dev);

//void i2c_wait(i2c_dev_t dev, uint8_t addr); 

uint8_t i2c_status(i2c_dev_t dev, uint16_t status); 

uint8_t i2c_open(i2c_dev_t dev); 

void i2c_close(i2c_dev_t dev); 


/**
 * Memory interface can be used for any storage medium
 */ 
 
struct memory_if; 
typedef struct memory_if **memory_dev_t; 

struct memory_if {
	/// writes a buffer into the interface
	/// returns number of bytes written
	/// always returns only when data has actually been written
	/// TODO: make it asynchronous, as with i2c_start_write
	size_t				(*write)(memory_dev_t self, size_t address, const uint8_t *data, size_t max_sz);
	/// reads a number of characters from device
	/// returns number of bytes read
	/// TODO: make it ascyncronous, as with i2c_start_read
	size_t				(*read)(memory_dev_t self, size_t address, uint8_t *data, size_t max_sz);
	
	/// reads device info and stores it in supplied variables
	void 					(*readInfo)(uint16_t *block_size, size_t *device_size); 
}; 

#define mem_write(mem, addr, data, size) (*mem)->write(mem, addr, data, size)
#define mem_read(mem, addr, data, size) (*mem)->read(mem, addr, data, size)

struct block_device; 
typedef struct block_device **block_dev_t; 

typedef uint16_t page_address_t; 

typedef enum {
	BLKDEV_BUSY = (1 << 0)
} blkdev_status_t; 

struct block_device_geometry {
	uint16_t page_size; 
	uint16_t pages; 
	uint16_t sectors; 
}; 

struct block_device {
	uint8_t (*open)(block_dev_t self); 
	int8_t 	(*close)(block_dev_t self); 
	ssize_t (*writepage)(block_dev_t self, page_address_t page, const uint8_t *data, ssize_t data_size); 
	ssize_t (*readpage)(block_dev_t self, page_address_t page, uint8_t *data, ssize_t data_size); 
	int8_t 	(*get_geometry)(block_dev_t self, struct block_device_geometry *geom); 
	uint8_t (*get_status)(block_dev_t self, blkdev_status_t flags); 
}; 

#define blk_open(dev) (*dev)->open(dev)
#define blk_close(dev) (*dev)->close(dev)
#define blk_writepage(dev, page, data, size) (*dev)->writepage(dev, page, data, size)
#define blk_readpage(dev, page, data, size) (*dev)->readpage(dev, page, data, size)
#define blk_get_geometry(dev, geom) (*dev)->get_geometry(dev, geom)
#define blk_get_status(dev, flags) (*dev)->get_status(dev, flags)

/**
 * PWM interface is for controlling pwm hardware. Usually it would be built in, timer
 * driven pwm channels, but the system does not limit you to just on board pwm channel
 * You could just as easily have a driver for an i2c or spi pwm controller export
 * this kind of interface and this would allow you to pass it to any component that
 * requires a pwm interface (such as for example a motor speed controller). 
 **/
struct pwm_interface {
	/// Sets channel "on" time to value specified in microseconds. A pwm controller can have
	/// several channels. Channel id is implementation specific id of the channel.
	/// returns actual duty cycle time that has been set. 
	uint16_t 		(*set_channel_duty)(struct pwm_interface *self, uint8_t channel, uint16_t value_us);
	/// Occasionally it may be beneficial to change period time of the pwm signal.
	/// Implementation should pick the prescaler value that gives the closest match to the
	/// specified period. This does not guarantee that the period value will be exact.
	/// but implementation should try to make it as exact as possible.
	/// returns actual period that has been set. 
	uint16_t 		(*set_channel_period)(struct pwm_interface *self, uint8_t channel, uint16_t value_us);
}; 

/**
 * Used for any device that can read an analog signal and report it as 16 bit value.
 * Since many adc peripherals have multiple channels, you specify which channel you want
 * to read. How the data is read is up to the implementation. Your actual driver may for
 * example use adc interrupt and read adc in the background and load it into a cache,
 * making your read_pin call return directly. Others may block until the adc value is
 * converted.
 **/
struct analog_interface {
	/// must return the value of the pin left justified (meaning that the values are
	/// converted to 16 bit value regardless of the resolution of the adc). Most hardware
	/// allows you to left adjust the result in hardware. 
	int16_t 	(*read_pin)(struct analog_interface *self, uint8_t pin); 
}; 
