/*

  m2elinfo.c

  Display scrollable long text (based on "strlist")

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

void m2_el_info_set_ptr(uint8_t *ptr, uint8_t is_rom) M2_NOINLINE;
void m2_el_info_start(void) M2_NOINLINE;
void m2_el_info_ptr_inc(void) M2_NOINLINE;
uint8_t m2_el_info_goto_next_line_break(void) M2_NOINLINE;
uint8_t m2_el_info_get_lines(void) M2_NOINLINE;
void m2_el_info_goto_line(uint8_t line) M2_NOINLINE;
void m2_el_info_copy_line(void) M2_NOINLINE;


/* number of chars in the RAM buffer, must include the terminating '\0' character */
/* #define M2_INFO_LINE_LEN 40 */
char m2_el_info_line[M2_INFO_LINE_LEN];
uint8_t m2_el_info_is_rom_ptr = 0;
uint8_t *m2_el_info_base_ptr;
uint8_t *m2_el_info_ptr;

uint8_t m2_el_info_cached_lines = 0;
uint8_t m2_el_info_last_pos = 0;
uint8_t *m2_el_info_last_ptr;

#ifdef M2_EL_CACHE_HIT_CNT
uint32_t m2_el_info_cache_hits = 0;
#endif 

void m2_el_info_set_ptr(uint8_t *ptr, uint8_t is_rom)
{
  if ( is_rom != 0 )
    if ( m2_el_info_base_ptr == ptr )
      return;
  m2_el_info_base_ptr = ptr;
  m2_el_info_is_rom_ptr = is_rom;
  m2_el_info_cached_lines = 255;
  m2_el_info_last_pos = 255;
}

void m2_el_info_start(void)
{
  m2_el_info_ptr = m2_el_info_base_ptr;
}

void m2_el_info_ptr_inc(void)
{
  m2_el_info_ptr++;
}

/* move m2_el_info_str_ptr to the next line break */
/* returns the terminating char which was found, \n or \0 */
uint8_t m2_el_info_goto_next_line_break(void)
{
  uint8_t c;
  uint8_t *ptr = m2_el_info_ptr;
  for(;;)
  {
    if ( m2_el_info_is_rom_ptr )
      c = m2_rom_low_level_get_byte(ptr);
    else
      c = *ptr;
    if ( c == '\n' )
      break;
    if ( c == '\0' )
      break;
    ptr++;
  }
  m2_el_info_ptr = ptr;
  return c;
}

/* count the number of lines */
/* m2_el_info_ptr must be set to the start of the string, before calling this procedure: m2_el_info_start() */
/* m2_el_info_ptr will point to the terminating \0 after calling this procedure */
uint8_t m2_el_info_get_lines(void)
{
  uint8_t lines = 1;
  
  if ( m2_el_info_is_rom_ptr != 0 )
    if ( m2_el_info_cached_lines != 255 )
    {
#ifdef M2_EL_CACHE_HIT_CNT
      m2_el_info_cache_hits++;
#endif
      return m2_el_info_cached_lines;
    }
  do
  {
    if ( m2_el_info_goto_next_line_break() == '\0' )
      break;
    m2_el_info_ptr_inc();
    lines++;
  } while( lines < ((1<<(sizeof(lines)*8))-2) );
  m2_el_info_cached_lines = lines;
  return lines;
}

/* count the number of lines */
/* m2_el_info_ptr must be set to the start of the string, before calling this procedure: m2_el_info_start() */
/* m2_el_info_ptr will point to the first char of the spezified line or \0  */
void m2_el_info_goto_line(uint8_t line)
{
  uint8_t pos = 0;

  if ( m2_el_info_is_rom_ptr != 0 )
    if ( m2_el_info_last_pos != 255 )
      if ( pos >= m2_el_info_last_pos )
      {
#ifdef M2_EL_CACHE_HIT_CNT
	m2_el_info_cache_hits++;
#endif
	pos = m2_el_info_last_pos;
	m2_el_info_ptr = m2_el_info_last_ptr;
      }
  
  for(;;)
  {
    if ( pos == line )
      break;
    if ( m2_el_info_goto_next_line_break() == '\0' )
      break;
    m2_el_info_ptr_inc();
    pos++;
  }
  
  m2_el_info_last_ptr =  m2_el_info_ptr;
  m2_el_info_last_pos = pos;
}

/* copy the line to the RAM buffer and add '\0' */
/* m2_el_info_ptr must be set to the start of the line, before calling this procedure: m2_el_info_start() */
void m2_el_info_copy_line(void)
{
  uint8_t pos;
  uint8_t c;
  uint8_t *ptr = m2_el_info_ptr;
  pos = 0;
  for(;;)
  {
    if ( m2_el_info_is_rom_ptr )
      c = m2_rom_low_level_get_byte(ptr);
    else
      c = *ptr;
    m2_el_info_line[pos] = c;
    if ( c == '\n' )
      break;
    if ( c == '\0' )
      break;
    if ( pos >= M2_INFO_LINE_LEN-1 )
      break;
    pos++;
    ptr++;
  }
  m2_el_info_line[pos] = '\0';
}

/* copy the line to the RAM buffer */
/* m2_el_info_ptr must be set to the start of the string, before calling this procedure: m2_el_info_start() */
static void m2_el_info_get_line(uint8_t line)
{
  m2_el_info_goto_line(line);
  m2_el_info_copy_line();
}

/*==============================================================*/
/* info structure access */

/* returned ptr might point to ram or rom */
static uint8_t *m2_el_info_get_ptr(m2_rom_void_p element)
{
  return (uint8_t *)m2_rom_get_ram_ptr(element, offsetof(m2_el_info_t, info_str));
}

static m2_button_fnptr m2_el_info_get_callback(m2_rom_void_p element)
{
	return (m2_button_fnptr)m2_rom_get_fnptr(element, offsetof(m2_el_infobase_t, select_callback));
}


/*==============================================================*/
/* info function */

static M2_EL_FN_DEF(m2_el_infoline_base_fn)
{
  uint8_t font;
  uint8_t pos;

  font = m2_el_parent_get_font(fn_arg->nav);
  pos = m2_nav_get_child_pos(fn_arg->nav);
  
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
        return 0;  /* not a list, return 0 */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_get_char_height_with_normal_border(font);
    case M2_EL_MSG_GET_WIDTH:
      return m2_el_slbase_calc_width(m2_nav_get_parent_element(fn_arg->nav));
    case M2_EL_MSG_NEW_FOCUS:
      m2_el_slbase_adjust_top_to_focus(m2_nav_get_parent_element(fn_arg->nav), pos);
      return 1;
    case M2_EL_MSG_SELECT:
    {
      m2_button_fnptr fn;
      fn = m2_el_info_get_callback(m2_nav_get_parent_element(fn_arg->nav));
      if ( fn != NULL )
	fn(fn_arg);
      /*
	the current line (starts at 0) can be calculated within fn by using
	  m2_nav_get_child_pos(fn_arg->nav);
	the total number of lines is returned by
	  m2_el_slbase_get_len(m2_nav_get_parent_element(fn_arg->nav))
      */
      return 1;
    }
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_el_slbase_calc_width((fn_arg->element));
	height = m2_el_slbase_calc_height((fn_arg->element));
	//m2_el_info_ptr = m2_el_info_get_ptr(m2_nav_get_parent_element(fn_arg->nav));
        m2_el_info_start();
	m2_el_info_get_line(pos);
	printf("infoline w:%d h:%d arg:%d x:%d y:%d '%s'\n", width, height, 
	    (fn_arg->arg), b->x, b->y, m2_el_info_line);
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
      //m2_el_info_ptr = m2_el_info_get_ptr(m2_nav_get_parent_element(fn_arg->nav));
      m2_el_info_start();
      m2_el_info_get_line(pos);
      m2_el_slbase_show(fn_arg, NULL, m2_el_info_line);
      return 1;
  }
  return 0;  
}

static M2_EL_FN_DEF(m2_el_infoline_fn)
{
  //m2_el_info_is_rom_ptr = 0;
  switch( fn_arg->msg )
  {
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
#endif
    case M2_EL_MSG_SHOW:
      m2_el_info_set_ptr(m2_el_info_get_ptr(m2_nav_get_parent_element(fn_arg->nav)), 0);
    break;
  }
  return m2_el_infoline_base_fn(fn_arg);
}

static M2_EL_FN_DEF(m2_el_infolinep_fn)
{
  //m2_el_info_is_rom_ptr = 1;
  switch( fn_arg->msg )
  {
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
#endif
    case M2_EL_MSG_SHOW:
      m2_el_info_set_ptr(m2_el_info_get_ptr(m2_nav_get_parent_element(fn_arg->nav)), 1);
    break;
  }
  return m2_el_infoline_base_fn(fn_arg);
}

m2_el_fnfmt_t m2_el_virtual_infoline M2_SECTION_PROGMEM = 
{
  m2_el_infoline_fn, NULL
};

m2_el_fnfmt_t m2_el_virtual_infolinep M2_SECTION_PROGMEM = 
{
  m2_el_infolinep_fn, NULL
};


static M2_EL_FN_DEF(m2_el_info_base_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      {
	uint8_t cnt;
	//m2_el_info_ptr = m2_el_info_get_ptr(fn_arg->element);
	m2_el_info_start();
	cnt = m2_el_info_get_lines();
	*m2_el_slbase_get_len_ptr(fn_arg->element) = cnt;
	return cnt;
      }
    case M2_EL_MSG_IS_AUTO_SKIP:
      return 1;
    case M2_EL_MSG_GET_OPT:
	if ( fn_arg->arg == 'd' )
	{
	  *(uint8_t *)(fn_arg->data) = 1;
	  return 1;
	}
	/* else... break out of the switch and let the base class do the rest of the work */
	break;
    case M2_EL_MSG_GET_LIST_BOX:
      return m2_el_slbase_calc_box(fn_arg->element, fn_arg->arg, ((m2_pcbox_p)(fn_arg->data)));
    case M2_EL_MSG_GET_HEIGHT:
      return m2_el_slbase_calc_height((fn_arg->element));
    case M2_EL_MSG_GET_WIDTH:
      return m2_el_slbase_calc_width((fn_arg->element));
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_el_slbase_calc_width((fn_arg->element));
	height = m2_el_slbase_calc_height((fn_arg->element));
	printf("info_p w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
	    (fn_arg->arg), b->x, b->y, m2_el_slbase_get_len(fn_arg->element));
      }
      return 0;
#endif
      case M2_EL_MSG_SHOW:
	/* MSG_SHOW: parent is drawn before the sub elements */
	/* adjust top element to total size, if required */
	m2_el_slbase_adjust_top_to_cnt(fn_arg->element);
	break;
  }
  return m2_el_fnfmt_fn(fn_arg);
}

M2_EL_FN_DEF(m2_el_info_fn)
{
  //m2_el_info_is_rom_ptr = 0;
  m2_el_info_set_ptr(m2_el_info_get_ptr(fn_arg->element), 0);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_infoline;
      return 1;
  }
  return m2_el_info_base_fn(fn_arg);
}

M2_EL_FN_DEF(m2_el_infop_fn)
{
  //m2_el_info_is_rom_ptr = 1;
  m2_el_info_set_ptr(m2_el_info_get_ptr(fn_arg->element), 1);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_infolinep;
      return 1;
  }
  return m2_el_info_base_fn(fn_arg);
}


