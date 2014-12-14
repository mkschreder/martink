/**
	Serial debugger module

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

/**
 * A serial debugger is a serial interface that forwards data you wire
 * into it to another serial interface and prints human readable version
 * of the data into a third debugger interface which can be a serial port. 
 **/

struct serial_debugger {
	/// connection to the device we will forward data to
	struct serial_interface *device; 
	/// connection to the debug console 
	struct serial_interface *console; 
	/// our serial interface that can be passed to another device
	struct serial_interface interface; 
	/// a line buffer to store bytes in (call flush to render all)
	uint8_t buffer[16]; 
	uint8_t buf_ptr; 
	uint8_t last_is_read; // 1 read 0 write
}; 

void serial_debugger_init(
	struct serial_debugger *dbg, 
	struct serial_interface *device, 
	struct serial_interface *console); 
uint16_t 	serial_debugger_getc(struct serial_interface *self);
uint16_t 	serial_debugger_putc(struct serial_interface *self, uint8_t ch);
size_t 		serial_debugger_putn(struct serial_interface *self, const uint8_t *data, size_t max_sz); 
size_t 		serial_debugger_getn(struct serial_interface *self, uint8_t *data, size_t max_sz);
void 			serial_debugger_flush(struct serial_interface *self);
size_t 		serial_debugger_waiting(struct serial_interface *self);

