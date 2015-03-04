/*

  m2align.c
  
  utility procedure for aligning (do not confuse with m2elalign.c which provides the align element)

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

uint8_t m2_align_get_max_height(m2_el_fnarg_p fn_arg, uint8_t size)
{
  uint8_t max;
  /* max = m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'h'); */
  max = m2_el_fnfmt_get_hH(fn_arg);
  if ( max < size )
    max = size;
  return max;
}

uint8_t m2_align_get_max_width(m2_el_fnarg_p fn_arg, uint8_t size)
{
  uint8_t max;
  /* max = m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'w'); */
  max = m2_el_fnfmt_get_wW(fn_arg);
  if ( max < size )
    max = size;
  return max;
}

/*
  centers a line of length "smaler_len" within a line of length "bigger_len"
  return 0 if bigger_len <= smaler_len
  else return (bigger_len - smaler_len) / 2
*/
uint8_t m2_get_center_line_offset(uint8_t bigger_len, uint8_t smaler_len)
{
  if ( bigger_len <= smaler_len )
    return 0;
  bigger_len -= smaler_len;
  bigger_len >>= 1;
  return bigger_len;
}