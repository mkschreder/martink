/**
	This file is part of martink project.

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


extern void 		twi0_init_default(void);

/// address is the first byte of data
void 		twi0_start_write(uint8_t addr, uint8_t *data, uint8_t data_sz);
/// address is the first byte of data
void 		twi0_start_read(uint8_t addr, uint8_t *data, uint8_t data_sz);

/// sends stop signal on the bus
void twi0_stop(void);

/// returns 1 if twi bus is processing another transaction
uint8_t twi0_busy(void);
