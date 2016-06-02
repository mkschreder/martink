/*

  m2ellisth.c

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
#include <assert.h>
#include <stdio.h>
#endif

/*
  calculate the size of the horizontal list except the border.
  The size is calculated by
    - adding the width of each child
    - finding the maximum of the height of all childs
    
  Note: Calls to this function can be nested
*/

static uint8_t m2_calc_hlist_width_overlap_correction(uint8_t width, uint8_t cnt)
{
  if ( cnt >= 2 )
  {
    cnt--;
    cnt *= m2_gfx_get_list_overlap_width();
    width -= cnt;
  }
  return width;
}

static uint8_t m2_calc_hlist_height(m2_rom_void_p element)
{
  return m2_expand_direction(element, m2_el_calc_child_fn(element, 0, m2_el_list_get_len_by_element(element), 1, 0), 'h' );
}

static uint8_t m2_calc_hlist_width(m2_rom_void_p element)
{
  uint8_t len = m2_el_list_get_len_by_element(element);
  return m2_expand_direction(element, m2_calc_hlist_width_overlap_correction(m2_el_calc_child_fn(element, 0, len, 0, 1), len), 'w' );
}


static void m2_calc_hlist_box(m2_rom_void_p element, uint8_t arg, m2_pcbox_p data)
{
  uint8_t len = m2_el_list_get_len_by_element(element);
  uint8_t target, min, delta;
  /* get min width */
  min = m2_calc_hlist_width_overlap_correction(m2_el_calc_child_fn(element, 0, len, 0, 1), len);
  /* get target width */
  target = m2_el_list_opt_get_val_zero_default(element, 'w');
  /* calculate lower space */
  delta = 0;
  if ( target > min )
  {
    delta = target;
    delta -= min;
    delta /= 2;
  }
  
  /* calculate new coordinates */
  data->c.x = delta;
  data->c.x += m2_calc_hlist_width_overlap_correction(m2_el_calc_child_fn(element, 0, arg, 0, 1), arg);
  data->c.x += data->p.x;
  data->c.y = data->p.y;  
#ifdef M2_EL_MSG_DBG_SHOW
  printf("- hlist calc box: idx:%d cx:%d cy:%d px:%d py:%d (delta:%d, argoverlapcorr:%d)\n", arg, data->c.x, data->c.y, data->p.x, data->p.y, delta,
    m2_calc_hlist_width_overlap_correction(m2_el_calc_child_fn(element, 0, arg, 0, 1), arg));
#endif
}

uint8_t m2_el_hlist_fn(m2_el_fnarg_p fn_arg)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_BOX:
      m2_calc_hlist_box(fn_arg->element, fn_arg->arg, (m2_pcbox_p)(fn_arg->data));
      return 1;  /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_calc_hlist_height((fn_arg->element));
    case M2_EL_MSG_GET_WIDTH:
      return m2_calc_hlist_width((fn_arg->element));
      /*
    case M2_EL_MSG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_calc_hlist_width(&(fn_arg->ae));
	height = m2_calc_hlist_height(&(fn_arg->ae));
	m2_gfx_box(b->x, b->y, width, height);
      }
      return 1;
    */
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_calc_hlist_width((fn_arg->element));
	height = m2_calc_hlist_height((fn_arg->element));
	printf("hlst w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
	      (fn_arg->arg), b->x, b->y, m2_el_list_get_len_by_element(fn_arg->element));
      }
      return 0;
#endif
  }
  return m2_el_listbase_fn(fn_arg);
}
