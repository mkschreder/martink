/*

  m2utls8d
  
  convert s8 to d

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

#include "m2utl.h"

/* UTL_STRING_CONVERSION_DATA_LEN is 6 */
/* extern char m2_utl_string_conversion_data[UTL_STRING_CONVERSION_DATA_LEN]; */



/* v = value, d = number of digits */

char *m2_utl_s8d(int8_t v, uint8_t d, uint8_t is_plus)
{
  char *p;
  uint8_t abs_v;
  if ( v >= 0 )
    abs_v = v;
  else
    abs_v = -v;
  
  d = 3-d;
  p = (char *)m2_utl_u8dp(m2_utl_string_conversion_data+1, abs_v) + d;
  p--;
  if ( v < 0 )
    *p = '-';
  else if ( v >= 0 )
  {
    if ( is_plus == 0 )
      *p = ' ';
    else
      *p = '+';
  }
  return p;
}


