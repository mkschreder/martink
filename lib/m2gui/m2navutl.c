/*
  
  m2navutl.c
  
  utility procedures for navigation: No other nav functions are called

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


    Code size status: 
      Proccedure attributes: Optimization completed
      Arithmetic optimication: Optimization completed
*/

#include "m2.h"

static m2_rom_void_p m2_nav_get_element_by_index(m2_nav_p nav, uint8_t depth) M2_NOINLINE;

m2_nav_p m2_get_nav(m2_p m2)
{
  return &((m2)->nav);
}

static m2_rom_void_p m2_nav_get_element_by_index(m2_nav_p nav, uint8_t depth)
{
  return nav->element_list[depth];
}

m2_rom_void_p m2_nav_get_current_element(m2_nav_p nav)
{
  return m2_nav_get_element_by_index(nav, nav->depth-1);
}

m2_rom_void_p m2_nav_get_parent_element(m2_nav_p nav)
{
  return m2_nav_get_element_by_index(nav, nav->depth-2);
}

void m2_nav_prepare_fn_arg_current_element(m2_nav_p nav)
{
  m2_fn_arg_clear();
  m2_fn_arg_set_nav(nav);
  m2_fn_arg_set_element(m2_nav_get_current_element(nav));
}

void m2_nav_prepare_fn_arg_parent_element(m2_nav_p nav)
{
  m2_fn_arg_clear();
  m2_fn_arg_set_nav(nav);
  m2_fn_arg_set_element(m2_nav_get_parent_element(nav));
}

/*
  send NEW_FOCUS message to the current element
*/
void m2_nav_send_new_focus(m2_nav_p nav)
{
  m2_nav_prepare_fn_arg_current_element(nav);
  m2_fn_arg_call(M2_EL_MSG_NEW_FOCUS);
}

/*
  check for read only element
*/
uint8_t m2_nav_is_read_only(m2_nav_p nav)
{
  m2_nav_prepare_fn_arg_current_element(nav);
  return m2_fn_arg_call(M2_EL_MSG_IS_READ_ONLY);
}

/*
  check for auto down (design element)
*/
uint8_t m2_nav_is_auto_skip(m2_nav_p nav)
{
  m2_nav_prepare_fn_arg_current_element(nav);
  return m2_fn_arg_call(M2_EL_MSG_IS_AUTO_SKIP);
}

uint8_t m2_nav_is_parent_auto_skip(m2_nav_p nav)
{
  if ( nav->depth < 2 )
    return 0;
  m2_nav_prepare_fn_arg_parent_element(nav);
  return m2_fn_arg_call(M2_EL_MSG_IS_AUTO_SKIP);
}

/*
  returns number of elements, for nav element at current nav depth
  return value 0: element does not have childs
*/
uint8_t m2_nav_get_list_len(m2_nav_p nav)
{
  m2_nav_prepare_fn_arg_current_element(nav);
  return m2_fn_arg_call(M2_EL_MSG_GET_LIST_LEN);
}

/*
  returns number of elements, for nav element at current nav depth
  return value 0: element does not have childs
*/
uint8_t m2_nav_get_parent_list_len(m2_nav_p nav)
{
  m2_nav_prepare_fn_arg_parent_element(nav);
  return m2_fn_arg_call(M2_EL_MSG_GET_LIST_LEN);
}

/*
  load child into element_list
  store pos into pos-array

  for the parent at depth-2, load a new child into depth-1, use the specified pos
  this procedure does not read or write nav->pos
*/
void m2_nav_load_child(m2_nav_p nav, uint8_t pos)
{
  /* load new child */
  m2_nav_prepare_fn_arg_parent_element(nav);
  m2_fn_arg_set_arg_data(pos, &(nav->element_list[nav->depth-1]));
  m2_fn_arg_call(M2_EL_MSG_GET_LIST_ELEMENT);
  nav->pos[nav->depth-2] = pos;
}

/* return the position number of the current child */
/* there is no error checking! */
uint8_t m2_nav_get_child_pos(m2_nav_p nav)
{
  uint8_t pos;
  
  /* depth is at least 2, because this is a virtual field of a text entry field */
  pos = nav->pos[nav->depth-2];  
  
  /* now the pointer to the char can be calulacted */
  return pos;
}

uint8_t m2_nav_is_data_entry(m2_nav_p nav)
{
  /* i think this code is obsolete */
  /*
  uint8_t pos = 0;
  if ( nav->depth >= 2 )
    pos = nav->pos[nav->depth-2];
  */
  m2_nav_prepare_fn_arg_current_element(nav);
  /*m2_fn_arg_set_arg_data(pos, NULL);*/
  return m2_fn_arg_call(M2_EL_MSG_IS_DATA_ENTRY);
}


