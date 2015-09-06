/*

  m2navupdn.c
  
  (auto) up/down procedure of the navigation subsystem

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



/*=========================================================================*/

/*
  Description:
    Navigate one level up, "goto parent". Used by draw algorithm.
    The root node will be removed.
  Return value:
    0, if the depth is 0 or 1.
  Side Effects: nav->depth
*/
uint8_t m2_nav_up(m2_nav_p nav)
{
  /*
  if ( nav->depth >= 2 )
    nav->pos[nav->depth-2] = 0xff;
  */
  if ( nav->depth <= 1 )
    return 0;
  nav->depth--;
  return 1;    
}


/*
  Description:
    Navigate one level down, "goto first child". Used by draw algorithm.
  Return value:
    0, if max depth reached or the current element is not a container
*/
uint8_t m2_nav_down(m2_nav_p nav, uint8_t is_msg)
{
  if ( m2_nav_get_list_len(nav) == 0 )
  {
    if ( is_msg != 0 )
    {
      m2_nav_prepare_fn_arg_current_element(nav);
      m2_fn_arg_set_arg_data(0, (void *)nav);
      return m2_fn_arg_call(M2_EL_MSG_SELECT);
    }
    return 0;
  }
  if ( nav->depth >= M2_DEPTH_MAX-1 )
    return 0;
  nav->depth++;
  m2_nav_load_child(nav, 0);
  return 1;
}




/*=========================================================================*/

/*
  as long as the parent has "auto skip" active, go to the parent.
  will only return 0 in case of an error.

  Side Effects: nav->depth
*/
uint8_t m2_nav_do_auto_up(m2_nav_p nav)
{
  if ( nav->depth <= 2 )
    return 1;
  while ( m2_nav_is_parent_auto_skip(nav) != 0 )
  {
    if ( m2_nav_up(nav) == 0 )
      return 0;
  }
  return 1;
}

uint8_t m2_nav_do_auto_down(m2_nav_p nav)
{
  while( m2_nav_is_auto_skip(nav) != 0 )
  {
    if ( m2_nav_down(nav, 0) == 0 )
    {
      return 0;
    }
  }
  return 1;
}

