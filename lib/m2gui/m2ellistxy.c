/*

  m2ellistxy.c
  
  place child at xy position

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
#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif


static uint8_t m2_get_xylist_element_x(m2_rom_void_p element)
{
  return m2_el_list_opt_get_val_zero_default(element, 'x');
}

static uint8_t m2_get_xylist_element_y(m2_rom_void_p element)
{
  return m2_el_list_opt_get_val_zero_default(element, 'y');
}

static uint8_t m2_get_xylist_element_x_plus_width(m2_rom_void_p element)
{
  return m2_get_xylist_element_x(element)+m2_fn_get_width(element);
}

static uint8_t m2_get_xylist_element_y_plus_height(m2_rom_void_p element)
{
  return m2_get_xylist_element_y(element)+m2_fn_get_height(element);
}

/*
  returns width for is_height == 0 
  returns height for is_height != 0
*/
static uint8_t m2_get_xylist_size(m2_rom_void_p element, uint8_t is_height)
{
  uint8_t i, len, max, val;
  m2_rom_void_p el;
  max = 0;
  len = m2_el_list_get_len_by_element(element);
  for( i = 0; i < len; i++ )
  {
    el = m2_el_list_get_child(element, i);
    if ( is_height == 0 )
      val = m2_get_xylist_element_x_plus_width(el);
    else
      val = m2_get_xylist_element_y_plus_height(el);
    if ( max < val )
      max = val;
  }
  return max;
}

static void m2_calc_xylist_box(m2_rom_void_p element, uint8_t arg, m2_pcbox_p data)
{
  m2_rom_void_p el;
  el = m2_el_list_get_child(element, arg);
  data->c.x = m2_get_xylist_element_x(el);
  data->c.y = m2_get_xylist_element_y(el);
  data->c.x += data->p.x;
  data->c.y += data->p.y;
}

uint8_t m2_el_xylist_fn(m2_el_fnarg_p fn_arg)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_BOX:
      m2_calc_xylist_box(fn_arg->element, fn_arg->arg, (m2_pcbox_p)(fn_arg->data));
      return 1;  /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_get_xylist_size((fn_arg->element), 1);
    case M2_EL_MSG_GET_WIDTH:
      return m2_get_xylist_size((fn_arg->element), 0);
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_get_xylist_size((fn_arg->element), 0);
	height = m2_get_xylist_size((fn_arg->element), 1);
	printf("xylst w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
	    (fn_arg->arg), b->x, b->y, m2_el_list_get_len_by_element(fn_arg->element));
      }
      return 0;
#endif
  }
  return m2_el_listbase_fn(fn_arg);
}

