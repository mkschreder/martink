/*

  m2qk.c
  
  quick key support

  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2013  olikraus@gmail.com

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

static m2_rom_void_p m2_quick_key_result_element;
static uint8_t m2_quick_key_search_value;

/* callback procedure for the DFS algorithm */
static uint8_t m2_find_quick_key(m2_nav_p nav)
{
  uint8_t element_quick_key = m2_el_fmfmt_opt_get_val_any_by_element(m2_nav_get_current_element(nav), 'q', 0);
  if ( element_quick_key == m2_quick_key_search_value )
  {
     m2_quick_key_result_element = m2_nav_get_current_element(nav);
    return 0;
  }

  return 1;
}



static uint8_t m2_qk_dfs_sub(m2_nav_p nav)
{
  if ( m2_find_quick_key(nav) == 0 )
    return 0;
  if ( m2_nav_down(nav, 0) != 0 )
  {
    do
    {
      if ( m2_qk_dfs_sub(nav) == 0 )
	return 0;
    } while ( m2_nav_next(nav) != 0 );
    m2_nav_up(nav);
  }
  return 1;
}



/*
  Search for an element with none-zero quick key. "quick_key_number" must not be 0.
  Quick key has a format option "q"
  Navigate to that element (update "nav")
  return 0 if quick key was not found, nav is not updated
  return 1 if quick key was found and nav has been updated

*/
uint8_t m2_nav_quick_key(m2_nav_p nav, uint8_t quick_key_value)
{
  m2_quick_key_result_element = NULL;
  m2_quick_key_search_value = quick_key_value;
  m2_nav_dfs(nav, m2_find_quick_key);
  if ( m2_quick_key_result_element != NULL )
  {
    while ( m2_nav_up(nav) != 0 )
      ;
    m2_qk_dfs_sub(nav);
    return 1;
  }
  return 0;
}

