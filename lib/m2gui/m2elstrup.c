/*

  m2elstrup.c
  
  go up button
  
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

M2_EL_FN_DEF(m2_el_str_up_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_SELECT:
	/* 2011-03-26: replaced m2_nav_up with m2_nav_user_up */
      	/* m2_nav_up((m2_nav_p)(fn_arg->data)); */
	m2_nav_user_up((m2_nav_p)(fn_arg->data));
      return 1;
  }
  return m2_el_str_fn(fn_arg);
}