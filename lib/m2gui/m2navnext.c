/*

  m2navnext.c

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
    Arithmetic optimization: Completed

*/

#include "m2.h"

/*
  Description:
    Navigate to next element. Used by draw algorithm.
  Return value:
    0, if max depth is 0 or 1 or if the current child has been the last child
*/
#ifdef OLD
uint8_t m2_nav_next(m2_nav_p nav)
{
  uint8_t len, pos;
  if ( nav->depth <= 1 )
    return 0;
  nav->depth--;
  len = m2_nav_get_list_len(nav);
  pos = nav->pos[nav->depth-1];
  pos++;
  if ( pos >= len )
  {
    /* restore original level */
    nav->depth++;
    return 0;
  }
  nav->depth++;
  m2_nav_load_child(nav, pos);
  return 1;
}
#endif

uint8_t m2_nav_next(m2_nav_p nav)
{
  uint8_t len, pos;
  uint8_t d;
  d = nav->depth;
  if ( d <= 1 )
    return 0;
  d-=2;
  len = m2_nav_get_parent_list_len(nav);
  pos = nav->pos[d];
  pos++;
  if ( pos >= len )
  {
    return 0;
  }
  m2_nav_load_child(nav, pos);
  return 1;
}
