/*
  me2elradio.c

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

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif

M2_EL_FN_DEF(m2_el_radio_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_gfx_get_icon_height(font, M2_ICON_RADIO_ACTIVE));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, m2_gfx_get_icon_width(font, M2_ICON_RADIO_ACTIVE));
    case M2_EL_MSG_SELECT:
      {
	uint8_t *ptr;
	ptr = m2_el_setval_get_val_ptr(fn_arg);
	*ptr = m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'v');
	return 1;
      }
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t *ptr;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	ptr = m2_el_setval_get_val_ptr(fn_arg);
	printf("radio arg:%d x:%d y:%d val:%d\n", 
	  (fn_arg->arg), 
	  b->x, b->y, 
	  *ptr);
      }
      return 0;
#endif
     case M2_EL_MSG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	uint8_t icon;
	uint8_t *ptr;

	/* get a pointer to the user value (which defines the active radio button) */
	ptr = m2_el_setval_get_val_ptr(fn_arg);
		
	/* handle focus and its border */
	if ( m2_is_frame_draw_at_end == 0 )
	  m2_el_setval_fn(fn_arg);		
	
	/* is the current radio button active */
	icon = M2_ICON_RADIO_INACTIVE;
	if ( *ptr == m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'v') )
	{
	  icon = M2_ICON_RADIO_ACTIVE; 
	}
	m2_gfx_draw_icon_add_normal_border_offset(b->x, b->y, font, icon);
	
	/* handle focus and its border */
	if ( m2_is_frame_draw_at_end != 0 )
	  m2_el_setval_fn(fn_arg);		
      }
      return 1;
  }
  return m2_el_setval_fn(fn_arg);
}


