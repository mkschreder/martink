/*

  m2elalign.c
  
  implements list element for aligning its child.

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


  supported options: h, w, -, |
  
  					xpos		ypos
    	center			-1		|1
    	upper middle		-1		|2
    	upper right		-2		|2
    	middle right		-2		|1
    	lower right		-2		|0
    	lower middle		-1		|0
    	lower left			-0		|0
    	middle left		-0		|1
    	upper left			-0		|2
    ...
    
*/

#include "m2.h"
#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif



m2_rom_void_p m2_el_align_get_element(m2_el_fnarg_p fn_arg)
{
  return (m2_rom_void_p)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_align_t, element));
}

/*
  return the overall dimension of a child, including all borders
*/
uint8_t m2_el_align_get_child_size(m2_el_fnarg_p fn_arg, uint8_t is_height)
{
  m2_rom_void_p element = m2_el_align_get_element(fn_arg);
  if ( is_height != 0 )
    return m2_fn_get_height(element);
  return m2_fn_get_width(element);
}

/*
use
uint8_t m2_align_get_max_size(m2_el_fnarg_p fn_arg, uint8_t size, uint8_t is_height)

*/
static uint8_t m2_align_get_new_size(m2_el_fnarg_p fn_arg, uint8_t is_height)
{
  uint8_t max, val;
  if ( is_height != 0 )
  {
    max = m2_el_fnfmt_get_hH(fn_arg);
    if ( max == 0 )
      max = m2_gfx_get_display_height();
  }
  else
  {
    max = m2_el_fnfmt_get_wW(fn_arg);
    if ( max == 0 )
      max = m2_gfx_get_display_width();
  }
  val = m2_el_align_get_child_size(fn_arg, is_height);
  if ( max < val )
    max = val;
  return max;
}

static uint8_t m2_align_get_pos(m2_el_fnarg_p fn_arg, uint8_t is_height, uint8_t pos)
{
  uint8_t d;
  
  if ( pos == 0 )
    return 0;
  
  d = m2_align_get_new_size(fn_arg, is_height);
  d -= m2_el_align_get_child_size(fn_arg, is_height);
  
  if ( pos == 2 )
    return d;
  
  d >>= 1;
  return d;
}

uint8_t m2_el_align_fn(m2_el_fnarg_p fn_arg)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 1;
    case M2_EL_MSG_IS_AUTO_SKIP:
      return 1;      
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = m2_el_align_get_element(fn_arg);
      return 1;
    case M2_EL_MSG_GET_LIST_BOX:
      ((m2_pcbox_p)(fn_arg->data))->c.y = m2_align_get_pos(fn_arg, 1, m2_opt_get_val(m2_el_fnfmt_get_fmt(fn_arg), '|'));
      ((m2_pcbox_p)(fn_arg->data))->c.x = m2_align_get_pos(fn_arg, 0, m2_opt_get_val(m2_el_fnfmt_get_fmt(fn_arg), '-'));
      ((m2_pcbox_p)(fn_arg->data))->c.y += ((m2_pcbox_p)(fn_arg->data))->p.y; 
      ((m2_pcbox_p)(fn_arg->data))->c.x += ((m2_pcbox_p)(fn_arg->data))->p.x; 
      return 1; /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_align_get_new_size(fn_arg, 1);
    case M2_EL_MSG_GET_WIDTH:
      return m2_align_get_new_size(fn_arg, 0);
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
        uint8_t h, w;
	      h = m2_align_get_new_size(fn_arg, 1);
	      w = m2_align_get_new_size(fn_arg, 0);
	      printf("align w:%d h:%d arg:%d \n", w, h, fn_arg->arg);
      }
      break;
#endif
      /* additional  border removed
    case M2_EL_MSG_SHOW:
      if ( fn_arg->arg > 0 )
      {
	uint8_t h, w;
	h = m2_align_get_new_size(fn_arg, 1);
	w = m2_align_get_new_size(fn_arg, 0);
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	m2_gfx_box(m2_gfx_add_normal_border_x(b->x), m2_gfx_add_normal_border_y(b->y), w, h);
      }
      return 1;
      */
  }
  return m2_el_fnfmt_fn(fn_arg);
}
