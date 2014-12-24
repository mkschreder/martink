/**
	8x8 led matrix display driver for 8 bit parallel port

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

	Special thanks to:
	* Davide Gironi, original implementation
*/

#ifndef LEDMATRIX88_H_
#define LEDMATRIX88_H_

#include <arch/interface.h>

struct ledmatrix88 {
	pio_dev_t port;
	uint8_t x_bank, y_bank;
	uint8_t data[8]; // pixel data
	uint8_t cur_x, cur_y; // for updates
};

//functions
extern void ledmatrix88_init(struct ledmatrix88 *self,
	pio_dev_t port, uint8_t x_bank, uint8_t y_bank);
extern void ledmatrix88_write_row(struct ledmatrix88 *self, uint8_t row, uint8_t data);
extern uint8_t ledmatrix88_read_row(struct ledmatrix88 *self, uint8_t row);
extern void ledmatrix88_clear(struct ledmatrix88 *self);
extern void ledmatrix88_update(struct ledmatrix88 *self);

#endif
