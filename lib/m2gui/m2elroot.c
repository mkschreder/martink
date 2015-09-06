/*
  
  m2elroot.c

  assign an element as new root
  based on the string element
  
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

#include <stddef.h>
#include "m2.h"

static m2_rom_void_p m2_el_root_get_ae(m2_el_fnarg_p fn_arg)
{
  return (m2_rom_void_p)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_root_t, element));
}

M2_EL_FN_DEF(m2_el_root_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_SELECT:
      m2_nav_set_root(fn_arg->nav,  m2_el_root_get_ae(fn_arg), m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'n'), m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'v'));
      return 1;
  }
  return m2_el_str_fn(fn_arg);
}


