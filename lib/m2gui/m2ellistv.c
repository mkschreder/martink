/*

  m2ellistv.c
  
  vertical list

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
#include <assert.h>
#endif

/*
  calculate the size of the vertical list except the border.
  The size is calculated by
    - adding the height of each child
    - finding the maximum of the width of all childs
    
  Note: Calls to this function can be nested
*/

uint8_t m2_calc_vlist_height_overlap_correction(uint8_t height, uint8_t cnt)
{
  if ( cnt >= 2 )
  {
    cnt--;
    cnt *= m2_gfx_get_list_overlap_height();
    height -= cnt;
  }
  return height;
}

static uint8_t m2_calc_vlist_height(m2_rom_void_p element)
{
  uint8_t len = m2_el_list_get_len_by_element(element);
  return m2_expand_direction(element, m2_calc_vlist_height_overlap_correction(m2_el_calc_child_fn(element, 0, len, 1, 1), len), 'h' );
}

static uint8_t m2_calc_vlist_width(m2_rom_void_p element)
{
  return m2_expand_direction(element, m2_el_calc_child_fn(element, 0, m2_el_list_get_len_by_element(element), 0, 0), 'w' );
}


/*
  this procedure uses the parent format do derive the height of each cell
  calculate the lower left position of the provided element (number in arg)

  example

    upper space
    000	idx 0   
    111	idx 1
    222	idx 2
    lower space

    min_height = m2_el_calc_child_fn(el, 0, el->len, 1, 1);
    target_height = m2_opt_get_val(fmt, 'h');

    if target_height > min_height 
      delta_height = target_height - min_height
    else
      delta_height = 0
    upper_space + lower_space = delta_height
    
    lower_space = delta_height / 2
    upper_height = delta_height - lower_space
*/


static void m2_calc_vlist_box(m2_rom_void_p element, uint8_t arg, m2_pcbox_p data)
{
  uint8_t len = m2_el_list_get_len_by_element(element);
  uint8_t target, min, delta;
  /* get min height */
  min = m2_calc_vlist_height_overlap_correction(m2_el_calc_child_fn(element, 0, len, 1, 1), len);
  /* get target height */
  target = m2_el_fnfmt_get_hH_by_element(element);
  /* calculate lower space */
  delta = 0;
  if ( target > min )
  {
    delta = target;
    delta -= min;
    delta /= 2;
  }
  
  /* calculate new coordinates */
  data->c.y = delta;
  data->c.y += min;
  arg++;
  data->c.y -= m2_calc_vlist_height_overlap_correction(m2_el_calc_child_fn(element, 0, arg, 1, 1), arg);
  data->c.y += data->p.y;
  data->c.x = data->p.x;
}

uint8_t m2_el_vlist_fn(m2_el_fnarg_p fn_arg)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_BOX:
      m2_calc_vlist_box(fn_arg->element, fn_arg->arg, (m2_pcbox_p)(fn_arg->data));
      return 1;  /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_calc_vlist_height((fn_arg->element));
    case M2_EL_MSG_GET_WIDTH:
      return m2_calc_vlist_width((fn_arg->element));
      /*
    case M2_EL_MSG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_calc_vlist_width(&(fn_arg->ae));
	height = m2_calc_vlist_height(&(fn_arg->ae));
	m2_gfx_box(b->x, b->y, width, height);
      }
      return 1;
    */
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_calc_vlist_width((fn_arg->element));
	height = m2_calc_vlist_height((fn_arg->element));
	printf("vlst w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
	    (fn_arg->arg), b->x, b->y, m2_el_list_get_len_by_element(fn_arg->element));
      }
      return 0;
#endif
  }
  return m2_el_listbase_fn(fn_arg);
}
