/*

  m2elsubutl.c

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

uint8_t m2_el_parent_get_font(m2_nav_p nav)
{
  /* check the argument: Some msg's do not provide the nav pointer */
  if ( nav == NULL )
    return 0;
  return m2_opt_get_val_zero_default(m2_el_fnfmt_get_fmt_by_element(m2_nav_get_parent_element(nav)), 'f');
}
