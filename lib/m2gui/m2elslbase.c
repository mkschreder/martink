/*

  m2elslbase.c

  base library for "strlist"

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
  
  
  slbase specific options:
    l:			Number of lines (defaults to 1)
  
*/

#include "m2.h"

/*==============================================================*/
/* slbase member access procedures */

uint8_t *m2_el_slbase_get_len_ptr(m2_rom_void_p element)
{
  return (uint8_t *)m2_rom_get_ram_ptr(element, offsetof(m2_el_slbase_t, len));
}

/* this is the total number of elements, do not confuse this with the visible line cnt */
uint8_t m2_el_slbase_get_len(m2_rom_void_p element)
{
  return *m2_el_slbase_get_len_ptr(element);
}


uint8_t *m2_el_slbase_get_top_ptr(m2_rom_void_p element)
{
  return (uint8_t *)m2_rom_get_ram_ptr(element, offsetof(m2_el_slbase_t, top));
}

uint8_t m2_el_slbase_get_top(m2_rom_void_p element)
{
  return *m2_el_slbase_get_top_ptr(element);
}


/*==============================================================*/
/* slbase extra column procedure */


/* return the extra font, defaults to the normal font given to 'f' */
static uint8_t m2_el_slbase_get_extra_font(m2_rom_void_p element)
{
  return m2_el_fmfmt_opt_get_val_any_by_element(element, 'F', m2_el_fmfmt_get_font_by_element(element));
}

/* width of the extra column */
static uint8_t m2_el_slbase_opt_get_eE(m2_rom_char_p str)
{
    uint8_t val;
    uint16_t tmp;
    val = m2_opt_get_val_zero_default(str, 'e'); 
    if ( val == 0 )
    {
      tmp = m2_opt_get_val_zero_default(str, 'E');
      tmp *= m2_gfx_get_display_width();
      tmp >>= 6;
      val = tmp;
    }  
    return val;
}

static uint8_t m2_el_slbase_eE_by_element(m2_rom_void_p element)
{
  return m2_el_slbase_opt_get_eE(m2_el_fnfmt_get_fmt_by_element(element));
}


/*==============================================================*/
/* slbase utility procedures */

/*
  return value from 0 .. cX-1 if the 
  based on the number of visible lines, the top visible element and the total number of lines, calculate if the 
  current (fn->arg) is visible. Also calc the position

  visible_lines  0...
  total_lines 0...
  top_line	0...total_lines-1
  curr_line (fn_arg->arg) 0...total_lines-1

  if ( curr_line < top_line )
    not visible
  if ( curr_line >= top_line + visible_lines )
    not_visible
  vis_pos = curr_line - top_line  
*/


/* argument must be of type m2_el_slbase_p */ 
uint8_t m2_el_slbase_get_visible_lines(m2_rom_void_p element)
{
  return m2_el_fmfmt_opt_get_val_any_by_element(element, 'l', 1);
}

/*
  Calculate the visible position, based on the internal index
  position. Order of the result is inverted, 0 is botton element.
  Return M2_EL_SLBASE_ILLEGAL if the value is not visible.
*/
/* argument must be of type m2_el_slbase_p */ 
uint8_t m2_el_slbase_get_visible_pos(m2_rom_void_p element, uint8_t idx)
{
  uint8_t top = m2_el_slbase_get_top(element);
  uint8_t lines;
  if ( idx < top )
    return M2_EL_SLBASE_ILLEGAL;
  idx -= top;
  lines = m2_el_slbase_get_visible_lines(element);
  if ( idx >= lines )
    return M2_EL_SLBASE_ILLEGAL;
  lines--;
  lines -= idx;
  return lines;
}

/* argument must be of type m2_el_slbase_p */ 
uint8_t m2_el_slbase_calc_height(m2_rom_void_p element)
{
  uint8_t h;
  uint8_t visible_lines = m2_el_slbase_get_visible_lines(element);
  h = m2_gfx_get_char_height_with_normal_border(m2_el_fmfmt_get_font_by_element(element));
  h *= visible_lines;
  h = m2_calc_vlist_height_overlap_correction(h, visible_lines);
  return h;
}

/* argument must be of type m2_el_slbase_p */ 
uint8_t m2_el_slbase_calc_width(m2_rom_void_p element)
{
  return m2_gfx_add_normal_border_width(m2_el_fmfmt_get_font_by_element(element), m2_el_fnfmt_get_wW_by_element(element))+m2_el_slbase_eE_by_element(element);
}

void m2_el_slbase_adjust_top_to_focus(m2_rom_void_p element, uint8_t pos)
{
  uint8_t cnt = m2_el_slbase_get_len(element);
  uint8_t top = m2_el_slbase_get_top(element);
  uint8_t lines;
  
  if ( pos < top )
  {
    top = pos;
  }
  else 
  {
    lines = m2_el_slbase_get_visible_lines(element);
    if ( pos >= top+lines )
    {
      top = pos;
      top -= lines;
      top++;
    }
    if ( top + lines >= cnt ) 
    {
      if ( cnt > lines )
      {
	top = cnt;
	top -= lines;
      }
      else
      {
	top = 0;
      }
    }
  }
  *m2_el_slbase_get_top_ptr(element) = top;
}

void m2_el_slbase_adjust_top_to_cnt(m2_rom_void_p element)
{
  uint8_t cnt = m2_el_slbase_get_len(element);
  uint8_t *top = m2_el_slbase_get_top_ptr(element);
  if ( *top >= cnt )
  {
    if ( cnt > 0 )
      cnt--;
    *top = cnt;
  }
}
 
/* 
  calculate pcbox for the callback procedures 
  return
    0: not visible
    1: visible
*/

uint8_t m2_el_slbase_calc_box(m2_rom_void_p el_slbase, uint8_t idx, m2_pcbox_p data)
{
  uint8_t visible_pos = m2_el_slbase_get_visible_pos(el_slbase, idx);
  uint8_t y;
  data->c.y = data->p.y;
  data->c.x = data->p.x;
  
  if ( visible_pos == M2_EL_SLBASE_ILLEGAL )
    return 0;
  
  y = m2_gfx_get_char_height_with_normal_border(m2_el_fmfmt_get_font_by_element(el_slbase));
  y *= visible_pos;
  visible_pos++;  /* for the overlap correction */
  y = m2_calc_vlist_height_overlap_correction(y, visible_pos);
  data->c.y += y;
#ifdef M2_EL_MSG_DBG_SHOW
  printf("- slbase calc box: idx:%d pos:%d cx:%d cy:%d px:%d py:%d\n", idx, visible_pos, data->c.x, data->c.y, data->p.x, data->p.y);
#endif
  return 1;
}

/*
  show the string on the display
*/

static void m2_gfx_draw_text2_add_normal_border_offset(uint8_t x0, uint8_t y0, uint8_t extra_font, const char *extra_s, uint8_t extra_offset, uint8_t font, const char *s)
{
  x0 = m2_gfx_add_normal_border_x(font, x0);
  y0 = m2_gfx_add_normal_border_y(font, y0);  
  if ( extra_s != NULL )
  {
    m2_gfx_text(x0, y0, 0, 0, extra_font, extra_s);
    x0 += extra_offset;
  }
  m2_gfx_text(x0, y0, 0, 0, font, s);
}

void m2_el_slbase_show(m2_el_fnarg_p fn_arg, const char *extra_s, const char *s)
{
  m2_pos_p b = (m2_pos_p)(fn_arg->data);
  uint8_t pos = m2_nav_get_child_pos(fn_arg->nav);
  uint8_t font = m2_el_parent_get_font(fn_arg->nav);
  uint8_t visible_pos = m2_el_slbase_get_visible_pos(m2_nav_get_parent_element(fn_arg->nav), pos);
  uint8_t extra_font = 0;
  uint8_t extra_offset = 0;
  
  if ( extra_s != NULL )
  {
    extra_font = m2_el_slbase_get_extra_font(m2_nav_get_parent_element(fn_arg->nav));
    extra_offset = m2_el_slbase_eE_by_element(m2_nav_get_parent_element(fn_arg->nav));
  }
  
  /* printf("STRLINE M2_EL_MSG_SHOW arg = %d, visible_pos = %d\n", fn_arg->arg, visible_pos); */
  
  if ( visible_pos != M2_EL_SLBASE_ILLEGAL )
  {
    uint8_t w, h;
    w = m2_el_slbase_calc_width(m2_nav_get_parent_element(fn_arg->nav));
    h = m2_gfx_get_char_height_with_normal_border(font);
    
    /*
      there is a sw architecture problem here: m2_el_fnfmt_fn() can not be called to create
      the focus/border, because the width of the element depends on the parent
    */
    if ( m2_is_frame_draw_at_end != 0 )
      m2_gfx_draw_text2_add_normal_border_offset(b->x, b->y, extra_font, extra_s, extra_offset, font, s);
      
    if ( fn_arg->arg < 2 )
      m2_gfx_normal_no_focus(b->x, b->y, w, h, font);      
    else if ( fn_arg->arg == 2 )
      m2_gfx_normal_focus(b->x, b->y, w, h, font);
    
    if ( m2_is_frame_draw_at_end == 0 )
      m2_gfx_draw_text2_add_normal_border_offset(b->x, b->y, extra_font, extra_s, extra_offset, font, s);
  }
}

