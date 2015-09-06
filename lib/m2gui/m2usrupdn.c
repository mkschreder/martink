/*

  m2usrupdn.c
  
  navigation procedure for the event handler: up and down

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


uint8_t m2_nav_user_up(m2_nav_p nav)
{
  uint8_t previous_depth = nav->depth;
  
  if ( m2_nav_do_auto_up(nav) == 0 )	/* m2navupdn.c */
    return 0;
  if ( m2_nav_up(nav) == 0 )
    return 0;
  /* m2_nav_do_auto_up() has ensured, that option a0 is now set, so only check if there are more parents. */
  
  if ( nav->depth < 2 )
  {
    nav->depth = previous_depth;	/* restore old depth, because there can't be more parents */
    return 0;
  }
  
  if ( m2_nav_get_parent_list_len(nav) == 0 )
  {
    nav->depth = previous_depth;	/* restore old depth, because parent is not a list */
    return 0;
  }

  m2_nav_send_new_focus(nav);
  return 1;
}

/*
  do down, but skip design elements.
  this procedure should be used by the event handlers
*/
uint8_t m2_nav_user_down(m2_nav_p nav, uint8_t is_msg)
{
  /* skip design elements */
  if ( m2_nav_do_auto_down(nav) == 0 )
    return 0;
  
  /* now go down */
  if ( m2_nav_down(nav, is_msg) == 0 )
    return m2_nav_do_auto_up(nav), 0;
  
  /* if read only, go next; if next fails, go up again and return 0 */
  if ( m2_nav_is_read_only(nav) != 0 )
    if ( m2_nav_user_next(nav) == 0 )
      return m2_nav_do_auto_up(nav), 0;
    
  m2_nav_send_new_focus(nav);
  return 1;
}



  
  