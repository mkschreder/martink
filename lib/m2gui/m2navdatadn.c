/*

  m2navdatadn.c

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

uint8_t m2_nav_data_down(m2_nav_p nav)
{
  uint8_t pos = 0;
  if ( nav->depth >= 2 )
    pos = nav->pos[nav->depth-2];
  m2_fn_arg_clear();
  m2_fn_arg_set_nav(nav);
  m2_fn_arg_set_element(m2_nav_get_current_element(nav));
  m2_fn_arg_set_arg_data(pos, NULL);
  return m2_fn_arg_call(M2_EL_MSG_DATA_DOWN);
}
