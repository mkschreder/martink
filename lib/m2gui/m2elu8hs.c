/*

  m2elu8hs.c

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

static uint8_t m2_el_u8_get_minmax(m2_el_fnarg_p fn_arg)
{
  uint8_t w;
  w = m2_el_u8_get_max(fn_arg);
  w += 1;
  w -= m2_el_u8_get_min(fn_arg);
  return w;
}

/*
  slide of size max-min+1
  has a one pixel border left and right
*/
static M2_EL_FN_DEF(m2_el_u8hs_sub_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, 5);
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, m2_el_u8_get_minmax(fn_arg)+2);
    case M2_EL_MSG_SHOW:
      {
	  m2_pos_p b = (m2_pos_p)(fn_arg->data);
	  m2_el_u8base_fn(fn_arg);

	  m2_gfx_hline(m2_gfx_add_normal_border_x(font, b->x+1), m2_gfx_add_normal_border_y(font, b->y+2), m2_el_u8_get_minmax(fn_arg));
	  m2_gfx_vline(m2_gfx_add_normal_border_x(font, b->x+(m2_el_u8_get_val(fn_arg))-m2_el_u8_get_min(fn_arg)+1), m2_gfx_add_normal_border_y(font, b->y), 5);
      }
      return 1;
  }
  return m2_el_u8base_fn(fn_arg);
}


M2_EL_FN_DEF(m2_el_u8hs_fn)
{
  return m2_el_u8hs_sub_fn(fn_arg);  
}
