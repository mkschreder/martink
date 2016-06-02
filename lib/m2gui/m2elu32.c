/*

  m2elu32.c
  
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
  
  
  get digit at pos
    while pos > 0
      u32 /= 10;
    u32 %= 10;
    
  Code size status: 
    Proccedure attributes: Optimization completed
    Arithmetic optimization: Mostly
  
  
*/


#include "m2.h"

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif


M2_EL_FN_DEF(m2_el_digit_fn);



uint32_t m2_el_u32_accumulator;
uint32_t m2_el_u32_pow10[9] = { 
1UL, 
10UL, 
100UL, 
1000UL, 
10000UL, 
100000UL, 
1000000UL, 
10000000UL, 
100000000UL };

/* returns the number of requested digits */
static uint8_t m2_el_u32_get_digits(m2_rom_void_p element) M2_NOINLINE;
static uint8_t m2_el_u32_get_digits(m2_rom_void_p element)
{
  uint8_t c = m2_opt_get_val_any_default(m2_el_fnfmt_get_fmt_by_element(element), 'c', 9);
  if ( c == 0 )
    c = 1;
  if ( c > 9 )
    c = 9;
  return c;
}

static uint8_t m2_el_u32_get_dot_pos(m2_rom_void_p element) M2_NOINLINE;
static uint8_t m2_el_u32_get_dot_pos(m2_rom_void_p element)
{
  uint8_t c = m2_el_u32_get_digits(element);
  uint8_t dot = m2_opt_get_val_zero_default(m2_el_fnfmt_get_fmt_by_element(element), '.');
  if ( dot > c )
    dot = 0;
  return dot;
}

/* returns the number of chars in the field, includes the dot */
static uint8_t m2_el_u32_get_char_size(m2_rom_void_p element)  M2_NOINLINE;
static uint8_t m2_el_u32_get_char_size(m2_rom_void_p element)
{
  uint8_t c = m2_el_u32_get_digits(element);
  uint8_t dot = m2_el_u32_get_dot_pos(element);
  if ( dot > 0 )
    c++;
  return c;
}

/*
static uint8_t m2_el_u32_get_parent_digits(m2_nav_p nav)
{
  return m2_el_u32_get_digits(m2_nav_get_parent_element(nav));
}
*/

static uint8_t m2_el_u32_get_parent_char_size(m2_nav_p nav) M2_NOINLINE;
static uint8_t m2_el_u32_get_parent_char_size(m2_nav_p nav)
{
  return m2_el_u32_get_char_size(m2_nav_get_parent_element(nav));
}

static uint8_t m2_el_u32_is_dot(m2_nav_p nav) M2_NOINLINE;
static uint8_t m2_el_u32_is_dot(m2_nav_p nav)
{
  uint8_t child_pos;
  uint8_t field_size;
  uint8_t dot_pos;
  uint8_t corrected_child_pos;
  
  dot_pos = m2_el_u32_get_dot_pos(m2_nav_get_parent_element(nav));
  if ( dot_pos == 0 )
    return 0;
  
  child_pos = m2_nav_get_child_pos(nav);
  field_size = m2_el_u32_get_parent_char_size(nav);
  corrected_child_pos = field_size;
  corrected_child_pos--;
  corrected_child_pos -= child_pos;
  if ( corrected_child_pos == dot_pos )
    return 1;
  return 0;
}

/* return the digit pos for the provided child */
static uint8_t m2_el_u32_get_digit_pos(m2_nav_p nav) M2_NOINLINE;
static uint8_t m2_el_u32_get_digit_pos(m2_nav_p nav)
{
  uint8_t dot_pos;
  uint8_t corrected_child_pos = m2_el_u32_get_parent_char_size(nav);
  uint8_t child_pos = m2_nav_get_child_pos(nav);
  
  corrected_child_pos--;
  corrected_child_pos -= child_pos;
  
  dot_pos = m2_el_u32_get_dot_pos(m2_nav_get_parent_element(nav));
  if ( dot_pos == 0 )
    return corrected_child_pos;
  if ( corrected_child_pos >= dot_pos )
    corrected_child_pos--;
  return corrected_child_pos;
}

static void m2_el_u32_set_accumulator_by_parent(m2_nav_p nav) M2_NOINLINE;
static void m2_el_u32_set_accumulator_by_parent(m2_nav_p nav)
{
  m2_rom_void_p element = m2_nav_get_parent_element(nav);
  if ( m2_rom_get_el_fnptr(element) == m2_el_u32fn_fn )
  {
    m2_u32fn_fnptr fn = (m2_u32fn_fnptr)m2_rom_get_fnptr(element, offsetof(m2_el_u32fn_t, u32_callback));
    m2_el_u32_accumulator = fn(element, M2_U32_MSG_GET_VALUE, 0);
  }
  else
  {
    m2_el_u32_accumulator = *(uint32_t *)m2_rom_get_ram_ptr(element, offsetof(m2_el_u32_t, val));
  }
}

static void m2_el_u32_put_accumulator_to_parent(m2_nav_p nav) M2_NOINLINE;
static void m2_el_u32_put_accumulator_to_parent(m2_nav_p nav)
{
  m2_rom_void_p element = m2_nav_get_parent_element(nav);
  if ( m2_rom_get_el_fnptr(element) == m2_el_u32fn_fn )
  {
    m2_u32fn_fnptr fn = (m2_u32fn_fnptr)m2_rom_get_fnptr(element, offsetof(m2_el_u32fn_t, u32_callback));
    if ( m2_el_u32_accumulator != fn(element, M2_U32_MSG_GET_VALUE, 0) )
      fn(element, M2_U32_MSG_SET_VALUE, m2_el_u32_accumulator);
  }
  else
  {
    *(uint32_t *)m2_rom_get_ram_ptr(element, offsetof(m2_el_u32_t, val)) = m2_el_u32_accumulator;
  }
}


/* accumulator -= num */
//static void m2_el_u32_sub(uint32_t num)  M2_NOINLINE;
static void m2_el_u32_sub(uint32_t num)
{
  m2_el_u32_accumulator -= num;
}

/* accumulator += num */
//static void m2_el_u32_add(uint32_t num) M2_NOINLINE;
static void m2_el_u32_add(uint32_t num)
{
  m2_el_u32_accumulator += num;
}

/* if accumulator >= num then true else false */
// static uint8_t m2_el_u32_cmp(uint32_t num) M2_NOINLINE;
static uint8_t m2_el_u32_cmp(uint32_t num)
{
  if ( m2_el_u32_accumulator >= num )
    return 1;
  return 0;
}

/* get the digit at the specified position from the accumulator */
/* force gcc not inline this function */
static uint8_t m2_el_u32_get_digit(uint8_t pos) M2_NOINLINE;
static uint8_t m2_el_u32_get_digit(uint8_t pos)
{
  uint8_t i = 8;
  uint8_t c;
  uint32_t pow10;
  uint32_t tmp = m2_el_u32_accumulator;
  for(;;)
  {
    c = 0;
    pow10 = m2_el_u32_pow10[i];
    
    while ( m2_el_u32_cmp(pow10)  != 0 )
    {
      c++;
      m2_el_u32_sub(pow10);
    }
    
    if ( i <= pos )
      break;    
    i--;
  }
  m2_el_u32_accumulator = tmp;
  return c;
}

//static void __m2_el_u32_inc_digit(uint8_t pos) M2_NOINLINE;
static void __m2_el_u32_inc_digit(uint8_t pos)
{
  m2_el_u32_add(m2_el_u32_pow10[pos]);
}

//static void __m2_el_u32_dec_digit(uint8_t pos) M2_NOINLINE;
static void __m2_el_u32_dec_digit(uint8_t pos)
{
  m2_el_u32_sub(m2_el_u32_pow10[pos]);
}

static void m2_el_u32_set_digit(uint8_t pos, uint8_t val)
{
  uint8_t d;
  d = m2_el_u32_get_digit(pos);
  while( val < d )
  {
    __m2_el_u32_dec_digit(pos);
    d--;
  }
  while( val > d )
  {
    __m2_el_u32_inc_digit(pos);
    d++;
  }

}

static void m2_el_u32_inc_digit(uint8_t pos)
{
  uint8_t d;
  d = m2_el_u32_get_digit(pos);
  if ( d < 9 )
  {
    __m2_el_u32_inc_digit(pos);
  }
  else
  {
    while( d > 0 )
    {
      __m2_el_u32_dec_digit(pos);
      d--;
    }
  }
}

static void m2_el_u32_dec_digit(uint8_t pos)
{
  uint8_t d;
  d = m2_el_u32_get_digit(pos);
  if ( d > 0 )
  {
    __m2_el_u32_dec_digit(pos);
  }
  else
  {
    while( d < 9 )
    {
      __m2_el_u32_inc_digit(pos);
      d++;
    }
  }
}

/* a special exit char allows leaving the text entry field */
/* the exit char is the last child of the text element */
/* argument nav points to the child */
static uint8_t m2_el_u32_is_exit_digit(m2_nav_p nav)
{
  register uint8_t len, pos;
  
  /* depth is at least 2, because this is a virtual field of a text entry field */
  pos = m2_nav_get_child_pos(nav);
  
  /* also, the parent must be the u32-element, return the "len" content */
  len = m2_el_u32_get_parent_char_size(nav);
  
  /* the number has "len" chars, but the parent has "len+1" children, so "pos" can be from 1 to "len" */
  if ( len == pos )
    return 1;
  
  return 0;
}

static void m2_el_u32_data_up(m2_nav_p nav)
{
  if ( m2_el_u32_is_dot(nav) )
    return;
  m2_el_u32_set_accumulator_by_parent(nav);
  m2_el_u32_inc_digit(m2_el_u32_get_digit_pos(nav));
  m2_el_u32_put_accumulator_to_parent(nav);
}

static void m2_el_u32_data_down(m2_nav_p nav)
{
  if ( m2_el_u32_is_dot(nav) )
    return;
  m2_el_u32_set_accumulator_by_parent(nav);
  m2_el_u32_dec_digit(m2_el_u32_get_digit_pos(nav));
  m2_el_u32_put_accumulator_to_parent(nav);
}

static void m2_el_u32_data_set(m2_nav_p nav, uint8_t msg)
{
  if ( msg >= '0' && msg <= '9' )
  {
    if ( m2_el_u32_is_dot(nav) )
      return;
    if ( m2_el_u32_is_exit_digit(nav) != 0 )
      return;
    msg -= '0';
    m2_el_u32_set_accumulator_by_parent(nav);
    m2_el_u32_set_digit(m2_el_u32_get_digit_pos(nav), msg);
    m2_el_u32_put_accumulator_to_parent(nav);
  }
}


M2_EL_FN_DEF(m2_el_digit_fn)
{
  uint8_t font;

  font = m2_el_parent_get_font(fn_arg->nav);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 0;  /* not a list, return 0 */
    case M2_EL_MSG_SELECT:
      m2_nav_user_up((m2_nav_p)(fn_arg->data));
      return 1;
    case M2_EL_MSG_IS_READ_ONLY:
      if ( m2_el_u32_is_dot(fn_arg->nav) )
        return 1;
      return 0;
    case M2_EL_MSG_IS_DATA_ENTRY:
      /* if this is the exit char, do not enter data entry mode */
      if ( m2_el_u32_is_exit_digit(fn_arg->nav) != 0 )
        return 0;
      /* otherwise, enter data entry mode */
      return 1;
    case M2_EL_MSG_DATA_UP:
      m2_el_u32_data_up(fn_arg->nav);
      return 1;
    case M2_EL_MSG_DATA_DOWN:
      m2_el_u32_data_down(fn_arg->nav);
      return 1;
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_get_char_height_with_small_border(font);
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_get_num_char_width_with_small_border(font);
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	uint8_t pos = m2_el_u32_get_digit_pos(fn_arg->nav);
	printf("chr arg:%d x:%d y:%d pos:%d\n", (fn_arg->arg), b->x, b->y, pos);
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
    {
	char s[2] = " ";
	uint8_t pos = m2_nav_get_child_pos(fn_arg->nav);
      	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	if (  m2_el_u32_is_exit_digit(fn_arg->nav) != 0 )
	{
	  if ( fn_arg->arg != 0 )
	  {
	    /* assumes, that pos is equal to the number of chars */
	    /* this is true... at the moment, see m2_is_exit_char() */
	    m2_gfx_go_up(b->x-pos*m2_gfx_get_num_char_width_with_small_border(font), b->y, pos*m2_gfx_get_num_char_width_with_small_border(font), m2_gfx_get_char_height_with_small_border(font), font);
	  }
	}
	else
	{
	  if ( m2_el_u32_is_dot(fn_arg->nav) )
	  {
	    s[0] = '.';
	  }	    
	  else
	  {
	    m2_el_u32_set_accumulator_by_parent(fn_arg->nav);
	    s[0] = m2_el_u32_get_digit(m2_el_u32_get_digit_pos(fn_arg->nav));
	    s[0] += '0';
	  }
	  
	  if ( m2_is_frame_draw_at_end != 0 )
	    m2_gfx_draw_text_add_small_border_offset(b->x, b->y, 0, 0, m2_el_parent_get_font(fn_arg->nav), s);
	  
	  if ( fn_arg->arg == 2 )
	  {
	    m2_gfx_small_focus(b->x, b->y, m2_gfx_get_num_char_width_with_small_border(font), m2_gfx_get_char_height_with_small_border(font), font);
	  }
	  else if ( fn_arg->arg == 3 )
	  {
	    m2_gfx_small_data_entry(b->x, b->y, m2_gfx_get_num_char_width_with_small_border(font), m2_gfx_get_char_height_with_small_border(font), font);
	  }
	  else
	  {
	    m2_el_fnfmt_fn(fn_arg);
	  }
	  
	  if ( m2_is_frame_draw_at_end == 0 )
	    m2_gfx_draw_text_add_small_border_offset(b->x, b->y, 0, 0, m2_el_parent_get_font(fn_arg->nav), s);
	}
      }
      return 1;
    default: /* handle numeric keys (KEY_0 ...) */
      m2_el_u32_data_set(fn_arg->nav, fn_arg->msg);
  }
  return 0;
}


/*
m2_el_digit_t m2_el_virtual_digit M2_SECTION_PROGMEM = 
{
  m2_el_digit_fn
};
*/

m2_el_fnfmt_t m2_el_virtual_digit M2_SECTION_PROGMEM = 
{
  m2_el_digit_fn, NULL
};

M2_EL_FN_DEF(m2_el_u32_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      {
	uint8_t cnt = m2_el_u32_get_char_size(fn_arg->element);
	if ( m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'a') == 0 )
	  cnt++;
	return cnt;
      }
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_digit;
      return 1;
    case M2_EL_MSG_IS_AUTO_SKIP:
      return m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'a'); 
    case M2_EL_MSG_GET_LIST_BOX:
      ((m2_pcbox_p)(fn_arg->data))->c.x=m2_gfx_add_normal_border_x(font, ((m2_pcbox_p)(fn_arg->data))->p.x + fn_arg->arg*m2_gfx_get_num_char_width_with_small_border(font));
      ((m2_pcbox_p)(fn_arg->data))->c.y=m2_gfx_add_normal_border_y(font, ((m2_pcbox_p)(fn_arg->data))->p.y);
      return 1;  /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_gfx_get_char_height_with_small_border(font));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, m2_el_u32_get_char_size(fn_arg->element)*m2_gfx_get_num_char_width_with_small_border(font));
    case M2_EL_MSG_GET_OPT:
	if ( fn_arg->arg == 'd' )
	{
	  *(uint8_t *)(fn_arg->data) = 1;
	  return 1;
	}
	/* else... break out of the switch and let the base class do the rest of the work */
	break;

#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	printf("u32 w:%d h:%d arg:%d x:%d y:%d len:%d\n", 
	  m2_fn_get_width((fn_arg->element)), 
	  m2_fn_get_height((fn_arg->element)), 
	  (fn_arg->arg), b->x, b->y, m2_el_u32_get_digits(fn_arg->element));
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
      if ( fn_arg->arg == 1 )
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	uint8_t w = m2_fn_get_width((fn_arg->element));
	uint8_t h = m2_fn_get_height((fn_arg->element));
	m2_gfx_normal_parent_focus(b->x, b->y, w, h, font);
	return 0;
      }
      break; 
      /* call fnfmt_fn() */
  }
  /* message M2_EL_MSG_SHOW is handled in m2_el_fnfmt_fn() */
  return m2_el_fnfmt_fn(fn_arg);
}

M2_EL_FN_DEF(m2_el_u32fn_fn)
{
  /* inside digit procedures, it is checked for the function address */
  return m2_el_u32_fn(fn_arg);
}
