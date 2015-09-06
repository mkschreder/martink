/*

  m2ellabelfn.c
  
  base class for any string elements

  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2012  olikraus@gmail.com

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

#include <stddef.h>
#include "m2.h"
#include <string.h>

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif


static m2_labelfn_fnptr m2_el_labelfn_get_callback(m2_el_fnarg_p fn_arg)
{
  return (m2_labelfn_fnptr)m2_rom_get_fnptr(fn_arg->element, offsetof(m2_el_labelfn_t, label_callback));
}

static const char *m2_el_labelfn_get_str(m2_el_fnarg_p fn_arg)
{
  return m2_el_labelfn_get_callback(fn_arg)(fn_arg->element);
}


M2_EL_FN_DEF(m2_el_labelfn_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 0;  /* not a list, return 0 */
    case M2_EL_MSG_IS_READ_ONLY:
      return 1;
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_readonly_border_height(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_readonly_border_width(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_labelfn_get_str(fn_arg))));
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	printf("labelfn w:%d h:%d arg:%d x:%d y:%d %s\n", 
	    m2_fn_get_width((fn_arg->element)), 
	    m2_fn_get_height((fn_arg->element)), 
	    fn_arg->arg, b->x, b->y, m2_el_labelfn_get_str(fn_arg));
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);

	if ( m2_is_frame_draw_at_end == 0 )
	  m2_el_fnfmt_fn(fn_arg);
	  
	m2_gfx_draw_text_add_readonly_border_offset(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), b->x, b->y, 
	  m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_labelfn_get_str(fn_arg))), 
	  m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)), 
	  font, m2_el_labelfn_get_str(fn_arg));
	
	if ( m2_is_frame_draw_at_end != 0 )
	  m2_el_fnfmt_fn(fn_arg);
	
      }
      return 1;    
  }
  return m2_el_fnfmt_fn(fn_arg);
}

