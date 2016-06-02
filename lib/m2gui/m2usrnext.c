/*

  m2usrnext.c
  
  navigation procedure for the event handler: next

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
    Arithmetic optimization: Done


*/

#include "m2.h"

static void m2_nav_user_down_first(m2_nav_p nav) M2_NOINLINE;
static uint8_t m2_nav_user_next_sub(m2_nav_p nav) M2_NOINLINE;

static void m2_nav_user_down_first(m2_nav_p nav)
{
  for(;;)
  {
    if ( m2_nav_get_list_len(nav) == 0 )
      break;
    if ( m2_nav_is_auto_skip(nav) == 0 )
      break;
    m2_nav_down(nav, 0);
  }
}

static uint8_t m2_nav_user_next_sub(m2_nav_p nav)
{
  for(;;)
  {
    if ( m2_nav_next(nav) != 0 )
    {
      m2_nav_user_down_first(nav);
      return 1;
    }
    else
    {
      if ( m2_nav_is_parent_auto_skip(nav) == 0 )
	return 0;
      if ( m2_nav_up(nav) == 0 )
	return 0;
    }
  }
}

uint8_t m2_nav_user_first(m2_nav_p nav)
{
  m2_nav_first(nav);
  m2_nav_user_down_first(nav);
  while ( m2_nav_is_read_only(nav) != 0 )
  {
    if ( m2_nav_user_next_sub(nav) == 0 )
      return 0;
  }
  m2_nav_send_new_focus(nav);
  return 1;
}

uint8_t m2_nav_user_next(m2_nav_p nav)
{
  for(;;)
  {
    if ( m2_nav_user_next_sub(nav) == 0 )
      return m2_nav_user_first(nav);
    if ( m2_nav_is_read_only(nav) == 0 )
      break;
  }
  m2_nav_send_new_focus(nav);
  return 1;
}

