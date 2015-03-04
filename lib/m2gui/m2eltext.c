/*

  m2eltext.c

  text entry
  

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
#include "m2utl.h"

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif

/*==============================================================*/
/* char element definition */

#define M2_EL_CHAR_MIN ' '
#define M2_EL_CHAR_MAX 'z'

/* obsolete, replaced by fnfmt */
/*
struct _m2_el_char_struct
{
  m2_el_fnptr fn;
};
typedef struct _m2_el_char_struct m2_el_char_t;
typedef struct _m2_el_char_struct *m2_el_char_p;
*/

M2_EL_FN_DEF(m2_el_char_fn);



/*==============================================================*/


static uint8_t m2_is_valid_text_char(uint8_t c)
{
  if ( c >= M2_EL_CHAR_MIN  && c <= M2_EL_CHAR_MAX )
    return 1;
  return 0;
}

/* only space and upper case */
/*
uint8_t m2_is_valid_text_char(uint8_t c)
{
  if ( c == ' ' || ( c >= 'A'  && c <= 'Z' ) )
    return 1;
  return 0;
}
*/

static uint8_t m2_get_next_valid_text_char(uint8_t c)
{
  uint8_t i = c;
  for(;;)
  {
    i++;
    if ( c == i )
      break;
    if ( m2_is_valid_text_char(i) != 0 )
      break;
  }
  return i;
}

static uint8_t m2_get_prev_valid_text_char(uint8_t c)
{
  uint8_t i = c;
  for(;;)
  {
    i--;
    if ( c == i )
      break;
    if ( m2_is_valid_text_char(i) != 0 )
      break;
  }
  return i;
}

/*==============================================================*/
/* char function */

static char *m2_get_char_ptr(m2_nav_p nav)
{
  uint8_t pos;
  char *text;
  
  /* depth is at least 2, because this is a virtual field of a text entry field */
  pos = nav->pos[nav->depth-2];
  
  /* also, the parent must be the text-element */
  text = (char *)m2_rom_get_ram_ptr(m2_nav_get_parent_element(nav), offsetof(m2_el_text_t, text));
  
  /* now the pointer to the char can be calulacted */
  return text + pos;
}

/* a special exit char allows leaving the text entry field */
/* the exit char is the last child of the text element */
static uint8_t m2_is_exit_char(m2_nav_p nav)
{
  uint8_t len, pos;
  
  /* depth is at least 2, because this is a virtual field of a text entry field */
  pos = nav->pos[nav->depth-2];
  
  /* also, the parent must be the text-element, return the "len" content */
  len = m2_rom_get_u8(m2_nav_get_parent_element(nav), offsetof(m2_el_text_t, len));
  
  /* the text has "len" chars, but the parent has "len+1" children, so "pos" can be from 1 to "len" */
  if ( len == pos )
    return 1;
  
  return 0;
}

M2_EL_FN_DEF(m2_el_char_fn)
{
  char *cp;
  uint8_t font;

  font = m2_el_parent_get_font(fn_arg->nav);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 0;  /* not a list, return 0 */
    case M2_EL_MSG_SELECT:
	m2_nav_user_up((m2_nav_p)(fn_arg->data));      
	return 1;
    case M2_EL_MSG_IS_DATA_ENTRY:
      /* if this is the exit char, do not enter data entry mode */
      if ( m2_is_exit_char(fn_arg->nav) != 0 )
	return 0;
      /* otherwise, enter data entry mode */
      return 1;
    case M2_EL_MSG_DATA_UP:
      cp = m2_get_char_ptr(fn_arg->nav);
      *cp = m2_get_next_valid_text_char(*cp);
      /*
      if ( *cp >= M2_EL_CHAR_MAX )
	*cp = M2_EL_CHAR_MIN;
      else
	(*cp)++;
      */
      return 1;
    case M2_EL_MSG_DATA_DOWN:
      cp = m2_get_char_ptr(fn_arg->nav);
      *cp = m2_get_prev_valid_text_char(*cp);
      /*
      if ( *cp <= M2_EL_CHAR_MIN )
	*cp = M2_EL_CHAR_MAX;
      else
	(*cp)--;
      */
      return 1;
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_get_char_height_with_small_border(font);
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_get_char_width_with_small_border(font);
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	uint8_t pos = m2_nav_get_child_pos(fn_arg->nav);
	printf("chr arg:%d x:%d y:%d pos:%d\n", (fn_arg->arg), b->x, b->y, pos);
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
    {
	    char s[2] = "1";
	    uint8_t pos = m2_nav_get_child_pos(fn_arg->nav);
      	m2_pos_p b = (m2_pos_p)(fn_arg->data);
      
	      if ( m2_is_exit_char(fn_arg->nav) != 0 )
	      {
	        /* assumes, that pos is equal to the number of chars */
	        /* this is true... at the moment, see m2_is_exit_char() */
	        if ( fn_arg->arg != 0 )
	        {
	          m2_gfx_go_up(
	            b->x-pos*m2_gfx_get_char_width_with_small_border(font), 
	            b->y, 
	            pos*m2_gfx_get_char_width_with_small_border(font), 
	            m2_gfx_get_char_height_with_small_border(font), font);
	        }
	      }
	      else
	      {
	        s[0] = *m2_get_char_ptr(fn_arg->nav);
	        
	        if ( m2_is_frame_draw_at_end != 0 )
	          m2_gfx_draw_text_add_small_border_offset(b->x, b->y, 0, 0, m2_el_parent_get_font(fn_arg->nav), s);

	        if ( fn_arg->arg == 2 )
	        {
	          m2_gfx_small_focus(b->x, b->y, m2_gfx_get_char_width_with_small_border(font), m2_gfx_get_char_height_with_small_border(font), font);
	        }
	        else if ( fn_arg->arg == 3 )
	        {
	          m2_gfx_small_data_entry(b->x, b->y, m2_gfx_get_char_width_with_small_border(font), m2_gfx_get_char_height_with_small_border(font), font);
	        }
	        else
	        {
                  m2_gfx_normal_no_focus(b->x, b->y, m2_gfx_get_char_width_with_small_border(font), m2_gfx_get_char_height_with_small_border(font), font);
	          //changed 27 mar 2012: m2_el_fnfmt_fn(fn_arg);
	        }
	        
	        if ( m2_is_frame_draw_at_end == 0 )
	          m2_gfx_draw_text_add_small_border_offset(b->x, b->y, 0, 0, m2_el_parent_get_font(fn_arg->nav), s);
	      }
      }
      return 1;
    default:	/* handle ascii codes */
      if ( fn_arg->msg >= M2_EL_MSG_SPACE ) 
      {	
	/* Handle ascii codes including M2_KEY_0 ... */
	/* The message code is identical to the ASCII code of the char */ 
	*m2_get_char_ptr(fn_arg->nav) = fn_arg->msg;
	return 1;
      }
      break;
  }
  return 0;
}

/*==============================================================*/

static uint8_t m2_el_text_get_len(m2_el_fnarg_p fn_arg)
{
  return m2_rom_get_u8(fn_arg->element, offsetof(m2_el_text_t, len));
}



/*==============================================================*/
/* text function */


m2_el_fnfmt_t m2_el_virtual_char M2_SECTION_PROGMEM = 
{
  m2_el_char_fn, NULL
};


M2_EL_FN_DEF(m2_el_text_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      {
	uint8_t cnt = m2_el_text_get_len(fn_arg);
	if ( m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'a') == 0 )
	  cnt++;
	return cnt;
      }
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_char;
      return 1;
    case M2_EL_MSG_IS_AUTO_SKIP:
      return m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'a'); 
    case M2_EL_MSG_GET_LIST_BOX:
      ((m2_pcbox_p)(fn_arg->data))->c.x=m2_gfx_add_normal_border_x(font, ((m2_pcbox_p)(fn_arg->data))->p.x + fn_arg->arg*m2_gfx_get_char_width_with_small_border(font));
      ((m2_pcbox_p)(fn_arg->data))->c.y=m2_gfx_add_normal_border_y(font, ((m2_pcbox_p)(fn_arg->data))->p.y);
      return 1;  /* element is always visible */
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_gfx_get_char_height_with_small_border(font));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, m2_el_text_get_len(fn_arg)*m2_gfx_get_char_width_with_small_border(font));
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
	printf("lst w:%d h:%d arg:%d x:%d y:%d len:%d\n", 
	  m2_fn_get_width((fn_arg->element)), 
	  m2_fn_get_height((fn_arg->element)), 
	  (fn_arg->arg), b->x, b->y, m2_el_text_get_len(fn_arg));
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
  /* M2_EL_MSG_SHOW handled here: */
  return m2_el_fnfmt_fn(fn_arg);
}

