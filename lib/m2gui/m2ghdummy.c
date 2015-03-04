/*

  m2ghdummy.c

  graphics dummy (null) handler

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
#include <string.h>

uint8_t m2_gh_dummy(m2_gfx_arg_p arg)
{
  switch(arg->msg)
  {
    case M2_GFX_MSG_GET_TEXT_WIDTH:
      return strlen(arg->s);
    case M2_GFX_MSG_GET_NUM_CHAR_WIDTH:
    case M2_GFX_MSG_GET_CHAR_WIDTH:
    case M2_GFX_MSG_GET_CHAR_HEIGHT:
    case M2_GFX_MSG_GET_ICON_WIDTH:
    case M2_GFX_MSG_GET_ICON_HEIGHT:
      return 1;
  }
  return 0;
}


