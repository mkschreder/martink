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

#include <stdio.h>
#include <string.h>

#include "ledmatrix88.h"

//define led matrix columns and rows
#define LEDMATRIX88_COLS 8
#define LEDMATRIX88_ROWS 8

volatile uint8_t ledmatrix88_col = 0; //contains column data
volatile uint8_t ledmatrix88_row = 0; //contains row data

// clear the display
void ledmatrix88_clear(struct ledmatrix88 *self) {
	memset(self->data, 0, sizeof(self->data)); 
}

void ledmatrix88_init(struct ledmatrix88 *self, pio_dev_t port, uint8_t x_bank, uint8_t y_bank) {
	self->port = port; 
	self->x_bank = x_bank;
	self->y_bank = y_bank;
	self->cur_x = self->cur_y = 0;
	memset(self->data, 0, sizeof(self->data)); 
}

void ledmatrix88_write_row(struct ledmatrix88 *self, uint8_t row, uint8_t data){
	self->data[row & 0x07] = data;
}

uint8_t ledmatrix88_read_row(struct ledmatrix88 *self, uint8_t row){
	return self->data[row & 0x07];
}

void ledmatrix88_update(struct ledmatrix88 *self) {
	//emit column data
	for(int row = 0; row < 8; row++){
		pio_write_word(self->port, self->x_bank, self->data[row]);
		pio_write_word(self->port, self->y_bank, (1 << row));
	}
}

