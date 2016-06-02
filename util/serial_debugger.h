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
	serial_dev_t device; 
	/// connection to the debug console 
	serial_dev_t console; 
	/// a line buffer to store bytes in (call flush to render all)
	uint8_t buffer[16]; 
	uint8_t buf_ptr; 
	uint8_t last_is_read; // 1 read 0 write

	struct serial_device_ops *_ex_serial; 
}; 

void serial_debugger_init(struct serial_debugger *dbg,
	serial_dev_t device, serial_dev_t console);
serial_dev_t serial_debugger_get_serial_interface(struct serial_debugger *self); 
