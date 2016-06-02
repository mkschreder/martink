/*

  m2elstrlist.c
  
  list of strings, one can be selected

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
  
  
  strlist specific options:
    w:		Width of the strlist box (required!)
    l:			Number of lines (defaults to 1)
  
  Notes:
    option 'h' is not supported, instead height is calculated by fontheight * number of lines
    option 'a' is NOT SUPPORTED and is always enabled

  TODO
    - change args to element --> Implemented
    - combo procedures, change to element --> not required any more, strlist does not use combo element any more

*/

#include "m2.h"

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif


/*==============================================================*/
/* strlist structure access */

static m2_strlist_cb_fnptr m2_el_strlist_cb_fnptr(m2_rom_void_p element)
{
  return (m2_strlist_cb_fnptr)m2_rom_get_fnptr(element, offsetof(m2_el_strlist_t, strlist_cb_fnptr));
}

static const char *m2_el_strlist_get_str(m2_rom_void_p element, uint8_t idx)
{
  return m2_el_strlist_cb_fnptr(element)(idx, M2_STRLIST_MSG_GET_STR);
}

static const char *m2_el_strlist_select(m2_rom_void_p element, uint8_t idx)
{
  return m2_el_strlist_cb_fnptr(element)(idx, M2_STRLIST_MSG_SELECT);
}

static const char *m2_el_strlist_get_extra_str(m2_rom_void_p element, uint8_t idx)
{
  return m2_el_strlist_cb_fnptr(element)(idx, M2_STRLIST_MSG_GET_EXTENDED_STR);
}




/*==============================================================*/
/* specific options */

/*==============================================================*/
/* strlist utility procedures */




/*==============================================================*/
/* strline */

static M2_EL_FN_DEF(m2_el_strline_fn)
{
  m2_rom_void_p parent_el = m2_nav_get_parent_element(fn_arg->nav);
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
      /* width is defined only be the eE and wW options */
      return m2_el_slbase_calc_width(parent_el);
    case M2_EL_MSG_GET_OPT:
      if ( fn_arg->arg == 't' )
      {
        /* child is touch sensitive if the parent is touch sensitive */
        *(uint8_t *)(fn_arg->data) = m2_el_fmfmt_opt_get_val_any_by_element(parent_el, fn_arg->arg, 0);
        return 1;
      }
      break;
    case M2_EL_MSG_NEW_FOCUS:
      /* adjust the top value, if required */
      m2_el_slbase_adjust_top_to_focus(parent_el, pos);
      return 1;
    case M2_EL_MSG_SELECT:
      /* make a call to the supplied user procedure */
      m2_el_strlist_select(parent_el, pos);
      return 1;
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
        uint8_t width, height;
        m2_pos_p b = (m2_pos_p)(fn_arg->data);
        width = m2_el_slbase_calc_width((fn_arg->element));
        height = m2_el_slbase_calc_height((fn_arg->element));
        printf("strline w:%d h:%d arg:%d x:%d y:%d\n", width, height, 
          (fn_arg->arg), b->x, b->y);
      }
      return 0;
#endif
    case M2_EL_MSG_SHOW:
     {
       const char *extra_str = NULL;
        if ( m2_opt_get_val( m2_el_fnfmt_get_fmt_by_element(parent_el), 'e') != M2_OPT_NOT_FOUND ||
          m2_opt_get_val( m2_el_fnfmt_get_fmt_by_element(parent_el), 'E') != M2_OPT_NOT_FOUND )
        {
          extra_str = m2_el_strlist_get_extra_str(parent_el, pos);
        }
        m2_el_slbase_show(fn_arg, extra_str, m2_el_strlist_get_str(parent_el, pos));
      }
      return 1;
  }
  return 0;
}


/*==============================================================*/
/* strlist function */

m2_el_fnfmt_t m2_el_virtual_strline M2_SECTION_PROGMEM = 
{
  m2_el_strline_fn, NULL
};

M2_EL_FN_DEF(m2_el_strlist_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return m2_el_slbase_get_len(fn_arg->element);
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_strline;
      return 1;
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
        printf("strlist w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
            (fn_arg->arg), b->x, b->y, m2_el_slbase_get_len(fn_arg->element));
      }
      return 0;
#endif
      case M2_EL_MSG_SHOW:
        /* MSG_SHOW: parent is drawn before the sub elements */
        /* adjust top element to total size, if required */
        m2_el_slbase_adjust_top_to_cnt(fn_arg->element);
        //fn_arg->arg = 0;  /* never draw focus for the parent element */
        break;
      case M2_EL_MSG_NEW_DIALOG:
        m2_el_strlist_cb_fnptr(fn_arg->element)(255, M2_STRLIST_MSG_NEW_DIALOG);
        return 0;
  }
  return m2_el_fnfmt_fn(fn_arg);
}



