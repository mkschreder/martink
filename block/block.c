/**
	Micro Block Device System 
	
	Copyright (c) 2016 Martin Schröder <mkschreder.uk@gmail.com>

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
	Github: https://github.com/mkschreder
*/

#include "block.h"
#include <kernel/list.h>

static LIST_HEAD(_block_devices); 

#include <serial/serial.h>
void block_register_device(struct block_device *dev){
	INIT_LIST_HEAD(&dev->list); 
	list_add_tail(&dev->list, &_block_devices); 	
}

struct block_device *block_get_device(int number){
	return list_get_entry(&_block_devices, number, struct block_device, list); 
}
