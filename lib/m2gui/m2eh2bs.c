/*

  m2eh2bs.c

  Event Handler
  
  2 Buttons: Next + Select
  Simplified data entry without data entry mode
  
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
    - Increment data on data entry fields
    
  NEXT:
    - Go to the next field
  

*/

#include "m2.h"

uint8_t m2_eh_2bs(m2_p ep, uint8_t msg, uint8_t arg1, uint8_t arg2)
{
	(void)arg1; 
	(void)arg2; 
  m2_nav_p nav = m2_get_nav(ep);
  switch(msg)
  {
    case M2_EP_MSG_SELECT:
      
      /* if the field has data entry, than increment the data */
      if ( m2_nav_is_data_entry(nav) )
	return m2_nav_data_up(nav);
      
      /* else go down: do not use m2_nav_down, instead use m2_nav_user_down to skip design elements */
      return m2_nav_user_down(nav, 1);
      
    case M2_EP_MSG_NEXT:
      return m2_nav_user_next(nav);
  }
  return 0;
}
