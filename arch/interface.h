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

#undef getc
#undef putc

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
 
struct serial_interface {
	/// getc reads one character from the interface
	/// returns UART_NO_DATA if none available
	uint16_t 			(*getc)(struct serial_interface *self); 
	/// putc writes a character into the interface
	/// waits for a timeout of 1-2 bytes if output buffer is full 
	/// discards data if tx buffer is full and no data can be sent
	/// Returns status of transaction. must return reasonably immidiately. 
	uint16_t 			(*putc)(struct serial_interface *self, uint8_t ch); 
	/// putn writes a buffer into the interface
	/// returns number of bytes written
	/// should return only when data has been at least copied into internal buffer of the driver
	/// is not required to wait until data has been transmitted on wire
	size_t				(*putn)(struct serial_interface *self, const uint8_t *data, size_t max_sz);
	/// reads a number of characters from device
	/// returns number of bytes read
	/// returns always without waiting for more data to arrive
	/// always works with internal rx buffer of the interface
	size_t				(*getn)(struct serial_interface *self, uint8_t *data, size_t max_sz);
	/// flushes the output buffer and returns only when completed
	/// must ensure that all data has been sent on wire
	void					(*flush)(struct serial_interface *self);
	/// returns current number of bytes available for reading
	size_t 				(*waiting)(struct serial_interface *self);
}; 

typedef struct serial_interface serial_t; 

#define s_putc(ptr_iface, ch) ptr_iface->putc(ptr_iface, ch)
#define s_getc(ptr_iface) ptr_iface->getc(ptr_iface)
#define s_putn(ptr_iface, data, size) ptr_iface->putn(ptr_iface, data, size)
#define s_getn(ptr_iface, data, size) ptr_iface->getn(ptr_iface, data, size)
#define s_flush(ptr_iface) ptr_iface->flush(ptr_iface)
#define s_waiting(ptr_iface) ptr_iface->waiting(ptr_iface)

/// status codes returned by methods of the interface
#define SERIAL_PARITY_ERROR			0x1000
#define SERIAL_FRAME_ERROR      0x0800      
#define SERIAL_OVERRUN_ERROR    0x0400        
/// returned by putc if outgoing buffer is full
#define SERIAL_BUFFER_FULL  		0x0200             
/// returned by getc if no data is available
#define SERIAL_NO_DATA          0x0100

#define DECLARE_SERIAL_INTERFACE(device_name) \
extern void 		PFDECL(device_name, flush, struct serial_interface *);\
extern size_t PFDECL(device_name, waiting, struct serial_interface *); \
extern uint16_t PFDECL(device_name, getc, struct serial_interface*);\
extern uint16_t PFDECL(device_name, putc, struct serial_interface*, uint8_t data);\
extern size_t 	PFDECL(device_name, putn, struct serial_interface*, const uint8_t *s, size_t c); \
extern size_t 	PFDECL(device_name, getn, struct serial_interface*, uint8_t *s, size_t c); 

//extern void PFDECL(device_name, puts, const char *s );

#define CONSTRUCT_SERIAL_INTERFACE(device_name) (struct serial_interface){\
	.getc = PFNAME(device_name, getc), \
	.putc = PFNAME(device_name, putc), \
	.getn = PFNAME(device_name, getn), \
	.putn = PFNAME(device_name, putn), \
	.flush = PFNAME(device_name, flush), \
	.waiting = PFNAME(device_name, waiting), \
}
