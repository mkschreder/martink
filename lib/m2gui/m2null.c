
/*

  m2navinit.c
  
  null element

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

static M2_EL_FN_DEF(m2_el_null_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_IS_READ_ONLY:
      return 1;
  }
  return m2_el_fnfmt_fn(fn_arg);
}

m2_el_fnfmt_t m2_null_element M2_SECTION_PROGMEM = { m2_el_null_fn, NULL };


