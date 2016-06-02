/*

  m2elsetval.c
  
  base class for radio and toggle elements

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

uint8_t *m2_el_setval_get_val_ptr(m2_el_fnarg_p fn_arg)
{
  return (uint8_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_setval_t, val));
}

M2_EL_FN_DEF(m2_el_setval_fn)
{
  uint8_t font;

  font = m2_el_fmfmt_get_font(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_HEIGHT:
      return m2_gfx_add_normal_border_height(font, m2_gfx_get_icon_height(0, M2_ICON_TOGGLE_ACTIVE));
    case M2_EL_MSG_GET_WIDTH:
      return m2_gfx_add_normal_border_width(font, m2_gfx_get_icon_width(0, M2_ICON_TOGGLE_ACTIVE));
 }
  return m2_el_fnfmt_fn(fn_arg);
}
