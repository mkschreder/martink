/*

  m2elcombofn.c
  
  selection between some string items

  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2013  olikraus@gmail.com

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


  Element:
    M2_COMBOFN(el,fmt,cnt,fnptr)

  fnptr:
    const char *m2_combofn_fnptr(m2_rom_void_p element, uint8_t msg, uint8_t *valptr)

  msg:
	M2_COMBOFN_MSG_GET_VALUE
	M2_COMBOFN_MSG_SET_VALUE
	M2_COMBOFN_MSG_GET_STRING
  

*/

#include <stddef.h>
#include <string.h>
#include "m2.h"
#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif

static uint8_t m2_el_combofn_get_len(m2_el_fnarg_p fn_arg)
{
  return m2_rom_get_u8(fn_arg->element, offsetof(m2_el_combofn_t, cnt));
}

static m2_combofn_fnptr m2_el_combofn_get_fnptr(m2_el_fnarg_p fn_arg)
{
  return (m2_combofn_fnptr)m2_rom_get_fnptr(fn_arg->element, offsetof(m2_el_combofn_t, fnptr));
}

static const char *m2_el_combofn_get_string(m2_el_fnarg_p fn_arg, uint8_t idx)
{
  return m2_el_combofn_get_fnptr(fn_arg)(fn_arg->element, M2_COMBOFN_MSG_GET_STRING, &idx);
}

static uint8_t m2_el_combofn_get_val(m2_el_fnarg_p fn_arg)
{
  uint8_t val;
  m2_el_combofn_get_fnptr(fn_arg)(fn_arg->element, M2_COMBOFN_MSG_GET_VALUE, &val);
  return val;
}

static void m2_el_combofn_set_val(m2_el_fnarg_p fn_arg, uint8_t val)
{
  m2_el_combofn_get_fnptr(fn_arg)(fn_arg->element, M2_COMBOFN_MSG_SET_VALUE, &val);
}

static uint8_t m2_el_combofn_get_max_len_idx(m2_el_fnarg_p fn_arg)
{
  uint8_t i, cnt, len, max, max_idx;
  cnt = m2_el_combofn_get_len(fn_arg);
  max = 0;
  max_idx = 0;
  for( i = 0; i < cnt; i++ )
  {
    len = strlen(m2_el_combofn_get_string(fn_arg, i));
    if ( max < len )
    {
      max = len;
      max_idx = i;
    }
  }
  return max_idx;
}

static void m2_el_combofn_inc(m2_el_fnarg_p fn_arg)
{
  uint8_t val = m2_el_combofn_get_val(fn_arg);
  val++;
  if ( val >= m2_el_combofn_get_len(fn_arg) )
    val = 0;
  m2_el_combofn_set_val(fn_arg, val);
}

static void m2_el_combofn_dec(m2_el_fnarg_p fn_arg)
{
  uint8_t val = m2_el_combofn_get_val(fn_arg);
  if ( val == 0 )
  {
	val = m2_el_combofn_get_len(fn_arg);
	if ( val > 0 )
	  val--;
  }
  m2_el_combofn_set_val(fn_arg, val);  
}

static const char *m2_el_combofn_get_curr_str(m2_el_fnarg_p fn_arg)
{
  return m2_el_combofn_get_string(fn_arg, m2_el_combofn_get_val(fn_arg));
}


M2_EL_FN_DEF(m2_el_combofn_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 0;  /* not a list, return 0 */
    case M2_EL_MSG_IS_DATA_ENTRY:
      return 1;
    case M2_EL_MSG_DATA_UP:
      m2_el_combofn_inc(fn_arg);
      return 1;
    case M2_EL_MSG_DATA_DOWN:
      m2_el_combofn_dec(fn_arg);
      return 1;
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, 
	    m2_align_get_max_width(fn_arg, 
	      m2_gfx_get_text_width(font,
		m2_el_combofn_get_string(fn_arg, 
		  m2_el_combofn_get_max_len_idx(fn_arg)))));
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	printf("combofn w:%d h:%d arg:%d x:%d y:%d max_idx %d\n", 
	    m2_fn_get_width((fn_arg->element)), 
	    m2_fn_get_height((fn_arg->element)), 
	    fn_arg->arg, b->x, b->y, m2_el_combofn_get_max_len_idx(fn_arg)
	    );
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
      {
		uint8_t max_width = m2_gfx_get_text_width(font,m2_el_combofn_get_string(fn_arg,m2_el_combofn_get_max_len_idx(fn_arg)));
	
		m2_pos_p b = (m2_pos_p)(fn_arg->data);
	
		if ( m2_is_frame_draw_at_end == 0 )
			m2_el_fnfmt_fn(fn_arg);
	
		m2_gfx_draw_text_add_normal_border_offset(b->x, b->y, 
	      m2_align_get_max_width(fn_arg, max_width),
	      m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)),
	      font, m2_el_combofn_get_curr_str(fn_arg));
	
		if ( m2_is_frame_draw_at_end != 0 )
			m2_el_fnfmt_fn(fn_arg);
      }
      return 1;    
  }
  return m2_el_fnfmt_fn(fn_arg);
}

