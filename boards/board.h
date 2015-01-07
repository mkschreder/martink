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

#include "interface.h"

#if defined(CONFIG_BOARD_MULTIWII)
#include "flight_control/multiwii.h"
#elif defined(CONFIG_BOARD_PROMINI_RXTX)
#include "promini_rftrx.h"
#elif defined(CONFIG_BOARD_ARDUINO_DUE)
#include "arduino_due.h"
#elif defined(CONFIG_BOARD_STM32F103)
#include "stm32f103.h"
#endif
