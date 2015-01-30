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
 * \file     aes256_dec.h
 * \email    daniel.otte@rub.de
 * \author   Daniel Otte
 * \date     2008-12-31
 * \license  GPLv3 or later
 * \ingroup  AES
 */

#ifndef AES256_DEC_H_
#define AES256_DEC_H_

#include "aes_types.h"
#include "aes_dec.h"

/**
 * \brief decrypt with 256 bit key.
 *
 * This function decrypts one block with the AES algorithm under control of
 * a keyschedule produced from a 256 bit key.
 * \param buffer pointer to the block to decrypt
 * \param ctx    pointer to the key schedule
 */
void aes256_dec(void* buffer, aes256_ctx_t* ctx);



#endif /* AES256_DEC_H_ */
