/*

  m2elstrptr.c
  
  base class for any string pointer elements

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

#include <stddef.h>
#include "m2.h"
#include <string.h>

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif


char *m2_el_strptr_get_str(m2_el_fnarg_p fn_arg)
{
  return *(char **)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_strptr_t, strptr));
  /*return (char *)m2_get_ae_ptr( &(fn_arg->ae), offsetof(m2_el_str_t, str));*/
}


M2_EL_FN_DEF(m2_el_strptr_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 0;  /* not a list, return 0 */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_strptr_get_str(fn_arg))));
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	printf("str w:%d h:%d arg:%d x:%d y:%d %s\n", 
	    m2_fn_get_width((fn_arg->element)), 
	    m2_fn_get_height((fn_arg->element)), 
	    fn_arg->arg, b->x, b->y, m2_el_strptr_get_str(fn_arg));
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	
	if ( m2_is_frame_draw_at_end == 0 )
	  m2_el_fnfmt_fn(fn_arg);
	
	m2_gfx_draw_text_add_normal_border_offset(b->x, b->y, 
	      m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_strptr_get_str(fn_arg))),
	      m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)),
	      font, m2_el_strptr_get_str(fn_arg));
	
	if ( m2_is_frame_draw_at_end != 0 )
	  m2_el_fnfmt_fn(fn_arg);
      }
      return 1;    
  }
  return m2_el_fnfmt_fn(fn_arg);
}

