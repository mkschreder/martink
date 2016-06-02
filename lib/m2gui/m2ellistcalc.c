/*

  m2ellistcalc.c

  universal calculation procedure for lists

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

/* 
  calculate over all childs 
    is_height == 0:	calculate over height
    is_height != 0:	calculate over width
    is_sum == 0:	calculate maximum
    is_sum != 0:		calculate sum
*/
uint8_t m2_el_calc_child_fn(m2_rom_void_p element, uint8_t start, uint8_t end, uint8_t is_height, uint8_t is_sum)
{
  uint8_t i;
  uint8_t s;
  uint8_t fn_s = 0;
  for( i = start; i < end; i++ )
  {
    s = m2_el_list_get_child_size(element, is_height, i);
    if ( is_sum != 0 )
    {
      fn_s += s;
    }
    else
    {
      if ( fn_s < s )
      {
	fn_s = s;
      }
    }
  }
  return fn_s;
}

/*
  simply does a 
  result = max(in, m2_opt_get_val_zero_default(fmt, optchar)
  optchar usually is 'h' or 'w'
*/
uint8_t m2_expand_direction(m2_rom_void_p element, uint8_t in, uint8_t optchar)
{
  uint8_t v;
  if ( optchar == 'w' )
    v  = m2_el_fnfmt_get_wW_by_element(element);
  else  if ( optchar == 'h' )
    v  = m2_el_fnfmt_get_hH_by_element(element);
  else
    v = m2_el_list_opt_get_val_zero_default(element, optchar);
  if ( in < v )
    return v;
  return in;
}
