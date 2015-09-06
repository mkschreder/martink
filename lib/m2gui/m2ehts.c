/*

  m2ehts.c

  Event Handler
  
  pure touch screen handler with select action on release
  
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


uint8_t m2_eh_ts(m2_p ep, uint8_t msg, uint8_t arg1, uint8_t arg2)
{
  // m2_nav_p nav = m2_get_nav(ep);
  switch(msg)
  {
    case M2_EP_MSG_TOUCH_PRESS:
      //puts("M2_EP_MSG_TOUCH_PRESS");
      m2_FindByXYM2(ep, arg1, arg2, 1 /* is_change_focus */,  0 /* is_send_select*/ );    
      return 1;
    
    case M2_EP_MSG_TOUCH_RELEASE:
      //puts("M2_EP_MSG_TOUCH_RELEASE");
      m2_FindByXYM2(ep, arg1, arg2, 1 /* is_change_focus */,  1 /* is_send_select*/ );    
      ep->element_focus = NULL;		/* disable focus */
      return 1;
  }
  return 0;
}


uint8_t m2_eh_4bsts(m2_p ep, uint8_t msg, uint8_t arg1, uint8_t arg2)
{
  if ( m2_eh_ts(ep, msg, arg1, arg2) != 0 )
    return 1;
  return m2_eh_4bks(ep, msg, arg1, arg2);
}

uint8_t m2_eh_6bsts(m2_p ep, uint8_t msg, uint8_t arg1, uint8_t arg2)
{
  if ( m2_eh_ts(ep, msg, arg1, arg2) != 0 )
    return 1;
  return m2_eh_6bks(ep, msg, arg1, arg2);
}

