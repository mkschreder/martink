/*

  m2el2lmenu.c
  
  implement a menu with expandable sub menues
  derived from slbase

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


	struct _m2_menu_entry
	{
	  const char *label;
	  m2_rom_void_p element;
	};
	typedef struct _m2_menu_entry m2_menu_entry;
	
	label:
		Name of the element. Treated as submenu if there is a '.' at
		the beginning of the string.
	element:
		This element is assigned as root element if the (sub-) menu 
		element is selected.


	struct _m2_xmenu_entry
	{
	  const char *label;
	  m2_rom_void_p element;
	  m2_strlist_cb_fnptr cb;
	};
	typedef struct _m2_xmenu_entry m2_xmenu_entry;

	label:
		Name of the element. Treated as submenu if there is a '.' at
		the beginning of the string.
		If label is the empty string "" or equal to "." then the callback
		procedure is called with a M2_STRLIST_MSG_GET_STR msg
		
	element
		Once selected, "element" is assigned as root element. 
		This root element can be overwritten by the callback 
		procedure cb
		
	cb:
		If cb is not NULL and the menu entry is selected, then 
		the the callback procedure cb is called with a 
		M2_STRLIST_MSG_SELECT message.

*/

#include "m2.h"

#ifdef M2_EL_MSG_DBG_SHOW
#include <stdio.h>
#endif

/*==============================================================*/
/* 2lmenu structure access */

static m2_menu_entry *m2_el_2lmenu_get_menu_data_ptr(m2_rom_void_p element)
{
  return (m2_menu_entry *)m2_rom_get_ram_ptr(element, offsetof(m2_el_2lmenu_t, menu_entries));
}

static m2_xmenu_entry *m2_el_x2lmenu_get_menu_data_ptr(m2_rom_void_p element)
{
  return (m2_xmenu_entry *)m2_rom_get_ram_ptr(element, offsetof(m2_el_2lmenu_t, menu_entries));
}

static uint8_t m2_el_2lmenu_get_menu_char(m2_rom_void_p element)
{
  return m2_rom_get_u8(element, offsetof(m2_el_2lmenu_t, menu_char));
}

static uint8_t m2_el_2lmenu_get_expanded_char(m2_rom_void_p element)
{
  return m2_rom_get_u8(element, offsetof(m2_el_2lmenu_t, expanded_char));
}

static uint8_t m2_el_2lmenu_get_submenu_char(m2_rom_void_p element)
{
  return m2_rom_get_u8(element, offsetof(m2_el_2lmenu_t, submenu_char));
}

/*==============================================================*/
/* cache variables */

m2_menu_entry *m2_el_2lmenu_data;
m2_xmenu_entry *m2_el_x2lmenu_data;
uint8_t m2_el_2lmenu_expanded_position = 255;
char m2_2lmenu_main_menu;
char m2_2lmenu_expanded_menu;
char m2_2lmenu_submenu;


/*==============================================================*/
/* 2lmenu list procedures */

static m2_rom_void_p m2_2lmenu_get_label(uint8_t defidx)
{
  return m2_el_2lmenu_data[defidx].label;
  // return m2_rom_get_rom_ptr(m2_2lmenu_data+defidx, offsetof(m2_menu_entry, label));
}

static m2_rom_void_p m2_2lmenu_get_element(uint8_t defidx)
{
  return m2_el_2lmenu_data[defidx].element;
  //return m2_rom_get_rom_ptr(m2_2lmenu_data+defidx, offsetof(m2_menu_entry, element));
}

/*==============================================================*/
/* x2lmenu list procedures */

static m2_rom_void_p m2_x2lmenu_get_label(uint8_t defidx)
{
  return m2_el_x2lmenu_data[defidx].label;
  // return m2_rom_get_rom_ptr(m2_2lmenu_data+defidx, offsetof(m2_menu_entry, label));
}

static m2_rom_void_p m2_x2lmenu_get_element(uint8_t defidx)
{
  return m2_el_x2lmenu_data[defidx].element;
  //return m2_rom_get_rom_ptr(m2_2lmenu_data+defidx, offsetof(m2_menu_entry, element));
}

static m2_strlist_cb_fnptr m2_x2lmenu_get_cb(uint8_t defidx)
{
  return m2_el_x2lmenu_data[defidx].cb;
  //return m2_rom_get_rom_ptr(m2_2lmenu_data+defidx, offsetof(m2_menu_entry, element));
}


/*==============================================================*/
/* 2lmenu procedures */


static uint8_t m2_2lmenu_is_submenu(uint8_t defidx)
{
  const char *label = (const char *)m2_2lmenu_get_label(defidx);
  if ( label == NULL )
    return 0;
  // if ( m2_rom_low_level_get_byte(label) != '.' )
  if ( label[0]   != '.' )
    return 0;
  return 1;
}

static uint8_t m2_2lmenu_has_submenu(uint8_t defidx)
{
  const char *label = (const char *)m2_2lmenu_get_label(defidx);
  if ( label == NULL )
    return 0;
  //if ( m2_rom_low_level_get_byte(label) == '.' )
  if ( label[0]   == '.' )
    return 0;
  return m2_2lmenu_is_submenu(defidx+1);
}

static uint8_t m2_2lmenu_get_submenu_cnt(uint8_t defidx)
{
  uint8_t cnt = 0;
  for(;;)
  {
    defidx++;
    if ( m2_2lmenu_is_submenu(defidx) == 0 )
      break;
    cnt++;
  }
  return cnt;
}

static uint8_t m2_2lmenu_get_defidx_by_strlistidx(uint8_t strlistidx)
{
  uint8_t strlistcnt = 0;
  uint8_t defidx = 0;
  for(;;)
  {
    if ( strlistidx == strlistcnt )
      break;
    if ( m2_2lmenu_get_label(defidx) == NULL )
      break;
    if ( m2_2lmenu_has_submenu(defidx) != 0 )
    {
      if ( defidx == m2_el_2lmenu_expanded_position )
      {
	defidx++;
	strlistcnt++;
	while(m2_2lmenu_is_submenu(defidx))
	{
	  if ( strlistidx == strlistcnt )
	    break;
	  defidx++;
	  strlistcnt++;
	}
      }
      else
      {
	defidx++;
	strlistcnt++;
	while(m2_2lmenu_is_submenu(defidx))
	{
	  defidx++;
	}
      }
    }
    else
    {
      defidx++;
      strlistcnt++;
    }
  }
  if ( strlistidx == 255 )
    return strlistcnt;
  return defidx;
}

/* calculate "cnt", based on "expanded" */
static void m2_2lmenu_update_cnt(m2_rom_void_p element)
{
  *m2_el_slbase_get_len_ptr(element) = m2_2lmenu_get_defidx_by_strlistidx(255);
}

/*==============================================================*/
/* x2lmenu procedures */


static uint8_t m2_x2lmenu_is_submenu(uint8_t defidx)
{
  const char *label = (const char *)m2_x2lmenu_get_label(defidx);
  if ( label == NULL )
    return 0;
  // if ( m2_rom_low_level_get_byte(label) != '.' )
  if ( label[0]   != '.' )
    return 0;
  return 1;
}

static uint8_t m2_x2lmenu_has_submenu(uint8_t defidx)
{
  const char *label = (const char *)m2_x2lmenu_get_label(defidx);
  if ( label == NULL )
    return 0;
  //if ( m2_rom_low_level_get_byte(label) == '.' )
  if ( label[0]   == '.' )
    return 0;
  return m2_x2lmenu_is_submenu(defidx+1);
}

static uint8_t m2_x2lmenu_get_submenu_cnt(uint8_t defidx)
{
  uint8_t cnt = 0;
  for(;;)
  {
    defidx++;
    if ( m2_x2lmenu_is_submenu(defidx) == 0 )
      break;
    cnt++;
  }
  return cnt;
}

static uint8_t m2_x2lmenu_get_defidx_by_strlistidx(uint8_t strlistidx)
{
  uint8_t strlistcnt = 0;
  uint8_t defidx = 0;
  for(;;)
  {
    if ( strlistidx == strlistcnt )
      break;
    if ( m2_x2lmenu_get_label(defidx) == NULL )
      break;
    if ( m2_x2lmenu_has_submenu(defidx) != 0 )
    {
      if ( defidx == m2_el_2lmenu_expanded_position )
      {
	defidx++;
	strlistcnt++;
	while(m2_x2lmenu_is_submenu(defidx))
	{
	  if ( strlistidx == strlistcnt )
	    break;
	  defidx++;
	  strlistcnt++;
	}
      }
      else
      {
	defidx++;
	strlistcnt++;
	while(m2_x2lmenu_is_submenu(defidx))
	{
	  defidx++;
	}
      }
    }
    else
    {
      defidx++;
      strlistcnt++;
    }
  }
  if ( strlistidx == 255 )
    return strlistcnt;
  return defidx;
}

/* calculate "cnt", based on "expanded" */
static void m2_x2lmenu_update_cnt(m2_rom_void_p element)
{
  *m2_el_slbase_get_len_ptr(element) = m2_x2lmenu_get_defidx_by_strlistidx(255);
}


/*==============================================================*/
/* strlist utility procedures */




/*==============================================================*/
/* 2lmenu line */

static M2_EL_FN_DEF(m2_el_2lmenu_line_fn)
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
    case M2_EL_MSG_NEW_FOCUS:
      /* adjust the top value, if required */
      m2_el_slbase_adjust_top_to_focus(parent_el, pos);
      return 1;
    case M2_EL_MSG_SELECT:
      {
	uint8_t defidx;
	defidx = m2_2lmenu_get_defidx_by_strlistidx(pos);
	m2_el_2lmenu_data = m2_el_2lmenu_get_menu_data_ptr(parent_el);
	if ( m2_2lmenu_has_submenu(defidx) != 0 )
	{
	  if ( m2_el_2lmenu_expanded_position == defidx )
	  {
	    m2_el_2lmenu_expanded_position = 255;
	    m2_2lmenu_update_cnt(parent_el);
	  }
	  else
	  {
	    if ( m2_el_2lmenu_expanded_position < defidx )
	    {
	      uint8_t cnt = m2_2lmenu_get_submenu_cnt(m2_el_2lmenu_expanded_position);
	      m2_el_2lmenu_expanded_position = defidx;
	      m2_2lmenu_update_cnt(parent_el);
	      while( cnt > 0 )
	      {
		cnt--;
		m2_HandleKeyM2(ep);				/* first, handle the existing keys */
		m2_SetKeyM2(M2_KEY_PREV);		/* put new key into queue, leave last added key in the queue */
	      }
	    }
	    else
	    {
	      m2_el_2lmenu_expanded_position = defidx;
	      m2_2lmenu_update_cnt(parent_el);
	    }
	  }
	}
	else
	{
	  m2_SetRootM2(ep, m2_2lmenu_get_element(defidx));
	}
      }
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
      m2_el_2lmenu_data = m2_el_2lmenu_get_menu_data_ptr(parent_el);
     {
       char extra_str[2] = " ";
	uint8_t defidx  = m2_2lmenu_get_defidx_by_strlistidx(pos);
	const char *ptr = m2_2lmenu_get_label(defidx);
       
	if ( m2_2lmenu_is_submenu(defidx) != 0 )
	  ptr++;
       
        if ( m2_opt_get_val( m2_el_fnfmt_get_fmt_by_element(parent_el), 'e') != M2_OPT_NOT_FOUND ||
          m2_opt_get_val( m2_el_fnfmt_get_fmt_by_element(parent_el), 'E') != M2_OPT_NOT_FOUND )
        {
	  if ( m2_el_2lmenu_expanded_position == defidx )
	  {
	    extra_str[0] = m2_el_2lmenu_get_expanded_char(parent_el);
	  }
	  else if ( m2_2lmenu_has_submenu(defidx) != 0 )
	  {
	    extra_str[0] = m2_el_2lmenu_get_menu_char(parent_el);
	  }
	  else if ( m2_2lmenu_is_submenu(defidx) != 0 )
	  {
	    extra_str[0] = m2_el_2lmenu_get_submenu_char(parent_el);
	  }
        }
        m2_el_slbase_show(fn_arg, extra_str, ptr);
      }
      return 1;
  }
  return 0;
}

/*==============================================================*/
/* 2lmenu line */

static M2_EL_FN_DEF(m2_el_x2lmenu_line_fn)
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
    case M2_EL_MSG_NEW_FOCUS:
      /* adjust the top value, if required */
      m2_el_slbase_adjust_top_to_focus(parent_el, pos);
      return 1;
    case M2_EL_MSG_SELECT:
      {
	uint8_t defidx;
	defidx = m2_x2lmenu_get_defidx_by_strlistidx(pos);
	m2_el_x2lmenu_data = m2_el_x2lmenu_get_menu_data_ptr(parent_el);
	if ( m2_x2lmenu_has_submenu(defidx) != 0 )
	{
	  if ( m2_el_2lmenu_expanded_position == defidx )
	  {
	    m2_el_2lmenu_expanded_position = 255;
	    m2_x2lmenu_update_cnt(parent_el);
	  }
	  else
	  {
	    if ( m2_el_2lmenu_expanded_position < defidx )
	    {
	      uint8_t cnt = m2_x2lmenu_get_submenu_cnt(m2_el_2lmenu_expanded_position);
	      m2_el_2lmenu_expanded_position = defidx;
	      m2_x2lmenu_update_cnt(parent_el);
	      while( cnt > 0 )
	      {
		cnt--;
		m2_HandleKey();				/* first, handle the existing keys */
		m2_SetKey(M2_KEY_PREV);		/* put new key into queue, leave last added key in the queue */
	      }
	    }
	    else
	    {
	      m2_el_2lmenu_expanded_position = defidx;
	      m2_x2lmenu_update_cnt(parent_el);
	    }
	  }
	}
	else
	{
	  {
	    m2_rom_void_p el = m2_x2lmenu_get_element(defidx);
	    if ( el != NULL )
	      m2_SetRoot(el);
	  }
	  {
	    m2_strlist_cb_fnptr cb = m2_x2lmenu_get_cb(defidx);
	    if ( cb != (m2_strlist_cb_fnptr)NULL )
	      cb(defidx, M2_STRLIST_MSG_SELECT);
	  }
	}
      }
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
      m2_el_x2lmenu_data = m2_el_x2lmenu_get_menu_data_ptr(parent_el);
     {
        char extra_str[2] = " ";
	uint8_t defidx  = m2_x2lmenu_get_defidx_by_strlistidx(pos);
	const char *ptr = m2_x2lmenu_get_label(defidx);
       
	if ( ptr != NULL )
	  if ( m2_x2lmenu_is_submenu(defidx) != 0 )
	    ptr++; 
	
	if ( ptr == NULL || ptr[0] == '\0' )
	{
	    m2_strlist_cb_fnptr cb = m2_x2lmenu_get_cb(defidx);
	    if ( cb != (m2_strlist_cb_fnptr)NULL )
	      ptr = cb(defidx, M2_STRLIST_MSG_GET_STR);
	}
       
        if ( m2_opt_get_val( m2_el_fnfmt_get_fmt_by_element(parent_el), 'e') != M2_OPT_NOT_FOUND ||
          m2_opt_get_val( m2_el_fnfmt_get_fmt_by_element(parent_el), 'E') != M2_OPT_NOT_FOUND )
        {
	  if ( m2_el_2lmenu_expanded_position == defidx )
	  {
	    extra_str[0] = m2_el_2lmenu_get_expanded_char(parent_el);
	  }
	  else if ( m2_x2lmenu_has_submenu(defidx) != 0 )
	  {
	    extra_str[0] = m2_el_2lmenu_get_menu_char(parent_el);
	  }
	  else if ( m2_x2lmenu_is_submenu(defidx) != 0 )
	  {
	    extra_str[0] = m2_el_2lmenu_get_submenu_char(parent_el);
	  }
        }
        m2_el_slbase_show(fn_arg, extra_str, ptr);
      }
      return 1;
  }
  return 0;
}


/*==============================================================*/
/* common procedure */

static M2_EL_FN_DEF(m2_el_2lmenu_common_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return m2_el_slbase_get_len(fn_arg->element);
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
    case M2_EL_MSG_NEW_DIALOG:
      /* moved to the specific menu procedures */
      /*  m2_el_2lmenu_data = m2_el_2lmenu_get_menu_data_ptr(fn_arg->element); */
      /* m2_2lmenu_update_cnt(fn_arg->element); */
      break;
    case M2_EL_MSG_NEW_FOCUS:
      /* obsolete, replaced by M2_EL_MSG_NEW_DIALOG */
      /*
      m2_el_2lmenu_data = m2_el_2lmenu_get_menu_data_ptr(fn_arg->element);
      m2_2lmenu_update_cnt(fn_arg->element);
      */
      break;
#ifdef M2_EL_MSG_DBG_SHOW
    case M2_EL_MSG_DBG_SHOW:
      {
	uint8_t width, height;
	m2_pos_p b = (m2_pos_p)(fn_arg->data);
	width = m2_el_slbase_calc_width((fn_arg->element));
	height = m2_el_slbase_calc_height((fn_arg->element));
	printf("2lmenu w:%d h:%d arg:%d x:%d y:%d len:%d\n", width, height, 
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



/*==============================================================*/
/* m2_el_2lmenu_fn function */

m2_el_fnfmt_t m2_el_virtual_2lmenu_line M2_SECTION_PROGMEM = 
{
  m2_el_2lmenu_line_fn, NULL
};

M2_EL_FN_DEF(m2_el_2lmenu_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_2lmenu_line;
      return 1;
    case M2_EL_MSG_NEW_DIALOG:
      m2_el_2lmenu_data = m2_el_2lmenu_get_menu_data_ptr(fn_arg->element);
      m2_2lmenu_update_cnt(fn_arg->element);
      break;
  }
  /* rest is done on the common procedure */
  return m2_el_2lmenu_common_fn(fn_arg);
}


/*==============================================================*/
/* x2lmenu function */

m2_el_fnfmt_t m2_el_virtual_x2lmenu_line M2_SECTION_PROGMEM = 
{
  m2_el_x2lmenu_line_fn, NULL
};

M2_EL_FN_DEF(m2_el_x2lmenu_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_ELEMENT:
      *((m2_rom_void_p *)(fn_arg->data)) = &m2_el_virtual_x2lmenu_line;
      return 1;
    case M2_EL_MSG_NEW_DIALOG:
      m2_el_x2lmenu_data = m2_el_x2lmenu_get_menu_data_ptr(fn_arg->element);
      m2_x2lmenu_update_cnt(fn_arg->element);
      break;
  }
  /* rest is done on the common procedure */
  return m2_el_2lmenu_common_fn(fn_arg);
}


