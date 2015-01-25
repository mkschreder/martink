/*
	This file is part of martink project.
	
	Implementation by Daniel Otte (daniel.otte@rub.de) Copyright (C) 2008  

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

	LibK project on github: https://github.com/mkschreder
*/
/**
 * \file     aes.h
 * \email    daniel.otte@rub.de
 * \author   Daniel Otte
 * \date     2008-12-30
 * \license  GPLv3 or later
 *
 */
#ifndef AES_H_
#define AES_H_

/*
#ifndef AVR
#define PROGMEM
#define pgm_read_byte(a) (*(a))
#endif
*/

#include <stdint.h>

#include <arch/soc.h>

#include "aes_types.h"
#include "aes128_enc.h"
#include "aes192_enc.h"
#include "aes256_enc.h"
#include "aes128_dec.h"
#include "aes192_dec.h"
#include "aes256_dec.h"
#include "aes_enc.h"
#include "aes_dec.h"
#include "aes_keyschedule.h"
#include "gf256mul.h"

#endif
