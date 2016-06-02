/*

  m2rom.c
  
  interface to access data in rom (rom abstraction layer)

  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2011  olikraus@gmail.com

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "string.h"
#include "m2.h"

/*==============================================================*/
/* low level access */

uint8_t m2_rom_low_level_get_byte(m2_rom_void_p ptr)
{
  uint8_t u8;
#if defined(M2_AVR_PGM)
  u8 = pgm_read_byte(ptr);
#else
  u8 = *(m2_rom_char_p)ptr;
#endif
  return u8;
}

void m2_rom_low_level_copy(void *dest, m2_rom_void_p src, uint8_t cnt)
{
#if defined(M2_AVR_PGM)
  memcpy_P(dest, src, cnt);
#else
  while( cnt > 0 )
  {
    *(uint8_t *)dest = *(const uint8_t *)src;
    dest = ((uint8_t *)dest) + 1;
    src = ((const uint8_t *)src) + 1;
    cnt--;
  }
#endif
}

void m2_rom_low_level_strncpy(void *dest, m2_rom_void_p src, uint8_t cnt)
{
#if defined(M2_AVR_PGM)
  strncpy_P(dest, src, cnt);
#else
  
  while( cnt > 0 )
  {
    *(uint8_t *)dest = *(const uint8_t *)src;
    if ( *(const uint8_t *)src == 0 )
      break;
    dest = ((uint8_t *)dest) + 1;
    src = ((const uint8_t *)src) + 1;
    cnt--;
  }
#endif
}

/*==============================================================*/
/* structure access */

static union m2_rom_tmp_area_union
{
  uint32_t u32;
  m2_rom_void_p rom_ptr;
  void *ram_ptr;
  m2_el_fnptr el_fnptr;
} m2_rom_tmp_area;

uint8_t m2_rom_get_u8(m2_rom_void_p base, uint8_t offset)
{
  return m2_rom_low_level_get_byte(((m2_rom_char_p)base)+offset);
}

int8_t m2_rom_get_s8(m2_rom_void_p base, uint8_t offset)
{
  return (int8_t)m2_rom_low_level_get_byte(((m2_rom_char_p)base)+offset);
}

uint32_t m2_rom_get_u32(m2_rom_void_p base, uint8_t offset)
{
  m2_rom_low_level_copy(&(m2_rom_tmp_area.u32),  ((m2_rom_char_p)base)+offset, sizeof(uint32_t));
  return m2_rom_tmp_area.u32;
}

m2_rom_void_p m2_rom_get_rom_ptr(m2_rom_void_p base, uint8_t offset)
{
  m2_rom_low_level_copy(&(m2_rom_tmp_area.rom_ptr),  ((m2_rom_char_p)base)+offset, sizeof(m2_rom_void_p));
  return m2_rom_tmp_area.rom_ptr;
}

void *m2_rom_get_ram_ptr(m2_rom_void_p base, uint8_t offset)
{
  m2_rom_low_level_copy(&(m2_rom_tmp_area.ram_ptr),  ((m2_rom_char_p)base)+offset, sizeof(void *));
  return m2_rom_tmp_area.ram_ptr;
}

/* the element function pointer is always the first member of the element data structure, so assume 0 for the offset */
m2_el_fnptr m2_rom_get_el_fnptr(m2_rom_void_p base)
{
  m2_rom_low_level_copy(&(m2_rom_tmp_area.el_fnptr),  ((m2_rom_char_p)base), sizeof(m2_el_fnptr));
  return m2_rom_tmp_area.el_fnptr;
}

/* return function pointer, treat it as el_fnptr, but could be any function pointer */
m2_el_fnptr m2_rom_get_fnptr(m2_rom_void_p base, uint8_t offset)
{
  m2_rom_low_level_copy(&(m2_rom_tmp_area.el_fnptr),  ((m2_rom_char_p)base)+offset, sizeof(m2_el_fnptr));
  return m2_rom_tmp_area.el_fnptr;
}


/*==============================================================*/
/* void ** access */

m2_rom_void_p m2_rom_get_ptr_list_rom_ptr(m2_rom_void_p base, uint8_t idx)
{
  m2_rom_low_level_copy(&(m2_rom_tmp_area.rom_ptr),  ((const m2_rom_void_p *)base)+idx, sizeof(m2_rom_void_p));
  return m2_rom_tmp_area.rom_ptr;  
}
