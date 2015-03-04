/*

  m2elfnfmt.c
  
  base function of all element procedures

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

#include "m2.h"

m2_rom_char_p m2_el_fnfmt_get_fmt_by_element(m2_rom_void_p element)
{
  return m2_rom_get_rom_ptr( element, offsetof(m2_el_fnfmt_t, fmt) );
}

m2_rom_char_p m2_el_fnfmt_get_fmt(const m2_el_fnarg_p fn_arg)
{
  return m2_el_fnfmt_get_fmt_by_element( fn_arg->element );
  /*return m2_rom_get_rom_ptr( fn_arg->ae.element, offsetof(m2_el_fnfmt_t, fmt) );*/
}

uint8_t m2_el_fmfmt_opt_get_val_zero_default(const m2_el_fnarg_p fn_arg, uint8_t c)
{
  return m2_opt_get_val_zero_default(m2_el_fnfmt_get_fmt(fn_arg), c);
}

uint8_t m2_el_fmfmt_opt_get_val_any_default(const m2_el_fnarg_p fn_arg, uint8_t c, uint8_t default_value)
{
  return m2_opt_get_val_any_default(m2_el_fnfmt_get_fmt(fn_arg), c, default_value);
}

uint8_t m2_el_fmfmt_opt_get_val_any_by_element(m2_rom_void_p element, uint8_t c, uint8_t default_value)
{
  return m2_opt_get_val_any_default(m2_el_fnfmt_get_fmt_by_element(element), c, default_value);
  
}

uint8_t m2_el_fmfmt_get_font(const m2_el_fnarg_p fn_arg)
{
  return m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'f');
}

uint8_t m2_el_fmfmt_get_font_by_element(m2_rom_void_p element)
{
  return m2_opt_get_val_zero_default(m2_el_fnfmt_get_fmt_by_element(element), 'f');
}

uint8_t m2_el_fmfmt_opt_get_a_one_default(const m2_el_fnarg_p fn_arg)
{
  return m2_opt_get_val_any_default(m2_el_fnfmt_get_fmt(fn_arg), 'a', 1);
}



/*
  This is the base procedure of all element procedures.
  It should be called by all other element procedures.
*/

M2_EL_FN_DEF(m2_el_fnfmt_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_IS_READ_ONLY:
      return m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'r');
    case M2_EL_MSG_GET_OPT:
	*(uint8_t *)(fn_arg->data) = m2_el_fmfmt_opt_get_val_zero_default(fn_arg, fn_arg->arg);
	return 1;
    case M2_EL_MSG_SHOW:
    {
    	uint8_t font;

    	font = m2_el_fmfmt_get_font(fn_arg);
      
    	m2_pos_p b = (m2_pos_p)(fn_arg->data);
    	uint8_t w = m2_fn_get_width(fn_arg->element);
    	uint8_t h = m2_fn_get_height(fn_arg->element);
	    if ( fn_arg->arg < 2 )
	      m2_gfx_normal_no_focus(b->x, b->y, w, h, font);      
	    else if ( fn_arg->arg == 2 )
	      m2_gfx_normal_focus(b->x, b->y, w, h, font);
	    else if ( fn_arg->arg == 3 )
	      m2_gfx_normal_data_entry(b->x, b->y, w, h, font);
    }
    return 1;
  }
  return 0;
}

/*==============================================================*/

uint8_t m2_opt_get_hH(m2_rom_char_p str)
{
    uint8_t val;
    uint16_t tmp;
    val = m2_opt_get_val_zero_default(str, 'h'); 
    if ( val == 0 )
    {
      tmp = m2_opt_get_val_zero_default(str, 'H');
      tmp *= m2_gfx_get_display_height();
      tmp >>= 6;
      val = tmp;
    }  
    return val;
}

uint8_t m2_el_fnfmt_get_hH(const m2_el_fnarg_p fn_arg)
{
  return m2_opt_get_hH(m2_el_fnfmt_get_fmt(fn_arg));
}

uint8_t m2_el_fnfmt_get_hH_by_element(m2_rom_void_p element)
{
  return m2_opt_get_hH(m2_el_fnfmt_get_fmt_by_element(element));
}

uint8_t m2_opt_get_wW(m2_rom_char_p str)
{
    uint8_t val;
    uint16_t tmp;
    val = m2_opt_get_val_zero_default(str, 'w'); 
    if ( val == 0 )
    {
      tmp = m2_opt_get_val_zero_default(str, 'W');
      tmp *= m2_gfx_get_display_width();
      tmp >>= 6;
      val = tmp;
    }  
    return val;
}

uint8_t m2_el_fnfmt_get_wW(const m2_el_fnarg_p fn_arg)
{
  return m2_opt_get_wW(m2_el_fnfmt_get_fmt(fn_arg));
}

uint8_t m2_el_fnfmt_get_wW_by_element(m2_rom_void_p element)
{
  return m2_opt_get_wW(m2_el_fnfmt_get_fmt_by_element(element));
}

