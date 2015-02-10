/**
	Macro based circular buffer implementation

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

#include "pp_math.h"

#define DECLARE_STATIC_CBUF(name, elem_type, size) \
static struct {\
	elem_type buffer[NEXT_POW2(size)];\
	int32_t head;\
	int32_t tail;\
	const int32_t total_size;\
	const int32_t size_mask; \
} name = { \
	.buffer = {0}, \
	.head = 0, \
	.tail = 0, \
	.total_size = NEXT_POW2(size), \
	.size_mask = (NEXT_POW2(size) - (1))\
};\

/// helper to get difference between head and tail counter
#define _cbuf_hmt(name) ((name)->head - (name)->tail)

/// evaluates to number of unread elements in buffer
#define cbuf_get_data_count(name) (\
	(_cbuf_hmt(name) < 0)?(_cbuf_hmt(name) + ((name)->total_size)):_cbuf_hmt(name)\
)

/// checks if buffer is empty
#define cbuf_is_empty(name) ((name)->head == (name)->tail)

/// tests if the buffer is full
#define cbuf_is_full(name) ((name)->head == (((name)->tail - 1) & (name)->size_mask))

/// reads next element from the buffer and updates pointers or -1 if no data
#define cbuf_get(name) ((!cbuf_is_empty(name))?((\
	(name)->tail = ((name)->tail + 1) & (name)->size_mask \
), (name)->buffer[(name)->tail]):(-1))

/// put a character into the buffer
/// evaluates to 0 if success, -1 if buffer full
#define cbuf_put(name, data) ((!cbuf_is_full(name))\
	?((name)->head = ((name)->head + 1) & (name)->size_mask, (name)->buffer[(name)->head] = (data) & 0xff, 0)\
	:(-1))
