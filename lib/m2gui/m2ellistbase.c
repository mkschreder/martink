/*

  m2ellistbase.c

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


/*
struct _m2_el_list_struct
{
  m2_fn_p fn;
  m2_pgm_char_t *fmt;
  uint8_t len;
  m2_attrib_element_p list;
};
*/


#include <stddef.h>
#include "m2.h"
#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif



/*=========================================================================*/

uint8_t m2_el_list_get_len_by_element(m2_rom_void_p element)
{
  return m2_rom_get_u8(element, offsetof(m2_el_list_t, len) );
}

uint8_t m2_el_list_get_len(m2_el_fnarg_p fn_arg)
{
  return m2_el_list_get_len_by_element(fn_arg->element);
}


/*=========================================================================*/

m2_rom_void_p m2_el_list_get_child(m2_rom_void_p element, uint8_t idx)
{
  const m2_rom_void_p *list_start_ptr;
  list_start_ptr = (const m2_rom_void_p *)m2_rom_get_rom_ptr(element, offsetof(m2_el_list_t, el_list));
  return m2_rom_get_ptr_list_rom_ptr(list_start_ptr, idx);
}

/*=========================================================================*/


/*
  get size of a child element
  size is 
    width (if is_height == 0 ) or
    height (if is_height != 0 )
*/
uint8_t m2_el_list_get_child_size(m2_rom_void_p element, uint8_t is_height, uint8_t idx)
{
  if ( is_height != 0 )
    return m2_fn_get_height(m2_el_list_get_child(element, idx));
  return m2_fn_get_width(m2_el_list_get_child(element, idx));  
}

/*=========================================================================*/


uint8_t m2_el_list_opt_get_val_zero_default(m2_rom_void_p element, char cmd)
{
  return m2_opt_get_val_zero_default(m2_el_fnfmt_get_fmt_by_element( element ), cmd);
}

/*=========================================================================*/


uint8_t m2_el_listbase_fn(m2_el_fnarg_p fn_arg)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_IS_AUTO_SKIP:
      return m2_el_fmfmt_opt_get_a_one_default(fn_arg); 
    case M2_EL_MSG_GET_LIST_LEN:
	return m2_el_list_get_len(fn_arg);
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = m2_el_list_get_child((fn_arg->element), fn_arg->arg);
      return 1;
  }
  return m2_el_fnfmt_fn(fn_arg);
}

