/*

  m2eh4bd.c
  
  Event Handler
  
  4 Buttons: Next/Prev + Select/Exit, 

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

  SELECT:
    - Execute actions on buttons
    - Go down one level in the element tree
    - Go up one level if the "go up" feature of a field is aktive
    - Switch between data entry 
    
  EXIT:
    - Go up
  
  NEXT:
    - Go to the next field
    - Increment data if in data entry mode

  PREV:
    - Go to the prev field
    - Decrement data if in data entry mode
    

*/

#include "m2.h"

uint8_t m2_eh_4bd(m2_p ep, uint8_t msg, uint8_t arg1, uint8_t arg2)
{
	(void)arg1; 
	(void)arg2; 
  m2_nav_p nav = m2_get_nav(ep);
  switch(msg)
  {
    case M2_EP_MSG_SELECT:
      /* if this is data entry mode, switch back */
      if ( nav->is_data_entry_active != 0 )
      {
	nav->is_data_entry_active = 0;
	return 1;
      }
      
      /* data entry mode not active; if we could enter data entry mode, then activate it */
      if ( m2_nav_is_data_entry(nav) )
      {
	nav->is_data_entry_active = 1;
	return 1;
      }
      
      /* go down: do not use m2_nav_down, instead use m2_nav_user_down to skip design elements */
      return m2_nav_user_down(nav, 1);

    case M2_EP_MSG_EXIT:
      return m2_nav_user_up(m2_get_nav(ep));
      
    case M2_EP_MSG_NEXT:
      if ( nav->is_data_entry_active != 0 )
	return m2_nav_data_up(m2_get_nav(ep));
      return m2_nav_user_next(m2_get_nav(ep));
      
    case M2_EP_MSG_PREV:
      if ( nav->is_data_entry_active != 0 )
	return m2_nav_data_down(m2_get_nav(ep));
      return m2_nav_user_prev(m2_get_nav(ep));
  }
  return 0;
}
