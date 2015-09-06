/*

  m2elu8num.c
  
  8 bit integer data input
  
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


  fn_arg->el_data:    Contains pointer to uint8_t value
  
*/

#include <stddef.h>
#include "m2.h"
#include "m2utl.h"

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif

static M2_EL_FN_DEF(m2_el_u8num_sub_fn)
{
  uint8_t digits;
  uint8_t width;
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);

  /* maybe the width is needed, precalculate the pixel width */
  
  /* get number of digits to show */
  digits = m2_opt_get_val_any_default(m2_el_fnfmt_get_fmt(fn_arg), 'c', 3);

  /* calculate pixel width */
  width = digits;
  width *= m2_gfx_get_num_char_width(font);

  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_gfx_get_char_height(font));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, width);
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	printf("u8  w:%d arg:%d x:%d y:%d %u\n", 
	  width, 
	  (fn_arg->arg), b->x, b->y, (unsigned)m2_el_u8_get_val(fn_arg));
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
    {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
      
	if ( m2_is_frame_draw_at_end == 0 )
	  m2_el_u8base_fn(fn_arg);
	
	m2_gfx_draw_text_add_normal_border_offset(b->x, b->y, 0, 0, font, m2_utl_u8d(m2_el_u8_get_val(fn_arg), digits));
	
	if ( m2_is_frame_draw_at_end != 0 )
	  m2_el_u8base_fn(fn_arg);
      }
      return 1;
  }
  return m2_el_u8base_fn(fn_arg);
}

M2_EL_FN_DEF(m2_el_u8num_fn)
{
  return m2_el_u8num_sub_fn(fn_arg);
}


M2_EL_FN_DEF(m2_el_u8numfn_fn)
{
  return m2_el_u8num_sub_fn(fn_arg);
}
