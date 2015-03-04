/*
  
  m2ellabelp.c
  
  a read-only string
  
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

M2_EL_FN_DEF(m2_el_labelp_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_IS_READ_ONLY:
      return 1;
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_readonly_border_height(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_readonly_border_width(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_width(fn_arg, m2_gfx_get_text_width_p(font,m2_el_str_get_str(fn_arg))));
    case M2_EL_MSG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);

	if ( m2_is_frame_draw_at_end == 0 )
	  m2_el_fnfmt_fn(fn_arg);
	
	m2_gfx_draw_text_p_add_readonly_border_offset(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), b->x, b->y, 
	  m2_align_get_max_width(fn_arg, m2_gfx_get_text_width_p(font,m2_el_str_get_str(fn_arg))), 
	  m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)), 
	  font, m2_el_str_get_str(fn_arg));
	
	if ( m2_is_frame_draw_at_end != 0 )
	  m2_el_fnfmt_fn(fn_arg);
      }
      return 1;    
  }
  return m2_el_str_fn(fn_arg);
}

