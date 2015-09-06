/*

  m2ellistgrid.c

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
/* option c contains the number of columns */

#include "m2.h"
#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif

static uint8_t m2_el_grid_get_cols(m2_rom_void_p element)
{
  uint8_t cols;
  
  /* fetch number of columns */
  cols = m2_el_list_opt_get_val_zero_default(element, 'c');

  if ( cols == 0 )
    cols++;
  
  return cols;
}

static uint8_t m2_el_grid_get_rows(m2_rom_void_p element)
{
  uint8_t cols;
  uint8_t rows;
  uint8_t tmp;
  
  /* fetch number of columns */
  cols = m2_el_grid_get_cols(element);
  
  /* calculate the number of rows */
  rows = 1;
  tmp = m2_el_list_get_len_by_element(element);
  while( tmp > cols )
  {
    rows++;
    tmp-=cols;
  }

  return rows;
}


/* return the height of the specified row */
static uint8_t m2_el_grid_get_row_height(m2_rom_void_p element, uint8_t y)
{
  uint8_t x;
  uint8_t cols;
  uint8_t h_max = 0;
  uint8_t h_cur = 0;
  uint8_t len = m2_el_list_get_len_by_element(element);
  
  cols = m2_el_grid_get_cols(element);
  
  y *= cols;
  
  for( x = 0; x < cols; x++, y++ )
  {
    if ( y < len )
    {
      h_cur = m2_el_list_get_child_size(element, 1, y);
      if ( h_max < h_cur )
	h_max = h_cur;
    }
  }
  return h_max;
}

/* return the width of the specified column */
static uint8_t m2_el_grid_get_col_width(m2_rom_void_p element, uint8_t x)
{
  uint8_t w_max = 0;
  uint8_t w_cur = 0;
  uint8_t cols;
  uint8_t len = m2_el_list_get_len_by_element(element);
  
  cols = m2_el_grid_get_cols(element);
    
  for(;;)
  {
    w_cur = m2_el_list_get_child_size(element, 0, x);
    if ( w_max < w_cur )
      w_max = w_cur;
    x += cols;
    if ( x >= len )
      break;
  }
  return w_max;
}


/*
  return width of the gridlist
*/
static uint8_t m2_calc_gridlist_width(m2_rom_void_p element)
{
  uint8_t cols  = m2_el_grid_get_cols(element);
  uint8_t i;
  uint8_t width;

  width = 0;
  for( i = 0; i < cols; i++ )
    width += m2_el_grid_get_col_width(element, i);

  /* use cols variable to calculate the overlap */
  cols--;
  cols *= m2_gfx_get_list_overlap_width();  
  width -= cols;
  
  return width;
}

/*
  return height of the gridlist
*/
static uint8_t m2_calc_gridlist_height(m2_rom_void_p element)
{
  uint8_t rows = m2_el_grid_get_rows(element);
  uint8_t i;
  uint8_t height;

  height = 0;
  for( i = 0; i < rows; i++ )
    height += m2_el_grid_get_row_height(element, i);

  /* use rows variable to calculate the overlap */
  rows--;
  rows *= m2_gfx_get_list_overlap_height();  
  height -= rows;
  
  return height;
}

/*
  for the specified child, calculate its lower left position
*/

static void m2_calc_gridlist_box(m2_rom_void_p element, uint8_t arg, m2_pcbox_p data)
{
  uint8_t cols  = m2_el_grid_get_cols(element);
  uint8_t rows = m2_el_grid_get_rows(element);
  uint8_t x, y;	
  uint8_t tmp;

  /* derive x and y coordinate of the child */
  x = arg;
  y = 0;  
  while( x >= cols )
  {
    x -= cols;
    y++;
  }
  /* x and y now contain the column and row of the child (starting with 0) */
  
  /* center the element according to its y position */
  tmp = m2_el_grid_get_row_height(element, y);
  tmp -= m2_el_list_get_child_size(element, 1, arg);
  tmp++;			/* round up if odd number */
  tmp >>= 1;
  
  /* lower left corner of the requested child, start with the y position */
  data->c.y = tmp;
  tmp = y;
  tmp++;
  if ( tmp < rows )
  {
    while( tmp < rows )
    {
      data->c.y += m2_el_grid_get_row_height(element, tmp);
      data->c.y -= m2_gfx_get_list_overlap_height();
      tmp++;
    }
    data->c.y += m2_gfx_get_list_overlap_height();
  }
  data->c.y += data->p.y;
  
  /* now calculate x position */
  data->c.x = 0;
  tmp = 0;
  if ( tmp < x )
  {
    while( tmp < x )
    {
      data->c.x += m2_el_grid_get_col_width(element, tmp);
      data->c.y -= m2_gfx_get_list_overlap_width();
      tmp++;
    }
    data->c.y += m2_gfx_get_list_overlap_width();
  }
  data->c.x += data->p.x;
  
  /* calculate the cell width and height */
  /*
  data->c.h = m2_el_grid_get_row_height(el, y);
  data->c.w = m2_el_grid_get_col_width(el, x);
  */
}

uint8_t m2_el_gridlist_fn(m2_el_fnarg_p fn_arg)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_BOX:
      m2_calc_gridlist_box((fn_arg->element), fn_arg->arg, (m2_pcbox_p)(fn_arg->data));
      return 1; /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_calc_gridlist_height(fn_arg->element);
    case M2_EL_MSG_GET_WIDTH:
      return m2_calc_gridlist_width(fn_arg->element);
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_calc_gridlist_width((fn_arg->element));
	height = m2_calc_gridlist_height((fn_arg->element));
	printf("lst w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
	  (fn_arg->arg), b->x, b->y, m2_el_list_get_len_by_element(fn_arg->element));
      }
      return 0;
#endif
  }
  return m2_el_listbase_fn(fn_arg);
}

