/*

  m2navroot.c
  
  Handle root element changes

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

/*=========================================================================*/
/* new root assignment */
/* idea: assign a new root to an intermediate variable, init the nav with the new root at top level */
/* any element function could call this procedure */
/* 1 Aug 2012: added next_cnt to allow the selection of a different inital field */

void m2_nav_set_root(m2_nav_p nav,  m2_rom_void_p element, uint8_t next_cnt, uint8_t change_value)
{
  if ( element == NULL )
    element = &m2_null_element;
  nav->new_root_element = element;
  nav->next_cnt = next_cnt;
  nav->root_change_value = change_value;
}

/* should be called only at the top level */
/* currently this is called in the event step procedure m2_ep_step() in m2ep.c */
uint8_t m2_nav_check_and_assign_new_root(m2_nav_p nav)
{
  if ( nav->new_root_element != NULL )
  {
    m2_rom_void_p new_root = nav->new_root_element;
    uint8_t cnt = nav->next_cnt;
    
    nav->new_root_element = NULL;					// make it zero, so others are allowed to make it none zero again.
    nav->next_cnt = 0;
    
    m2_nav_init(nav, new_root);
    
    while( cnt > 0 )
    {
      m2_nav_user_next(nav);
      cnt--;
    }
    
    return 1;
  }
  return 0;
}


