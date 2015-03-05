/*

  m2elhide.c
  
  changes visibility of a sub element.

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

static uint8_t *m2_el_hide_get_val_ptr(m2_el_fnarg_p fn_arg)
{
  return (uint8_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_hide_t, val));
}

uint8_t m2_el_hide_fn(m2_el_fnarg_p fn_arg)
{
  uint8_t val;
  val = *m2_el_hide_get_val_ptr(fn_arg);
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_LIST_LEN:
      return 1; 
    case M2_EL_MSG_IS_AUTO_SKIP:
      if ( val >= 1 )
        return 0;
      return 1; 
    case M2_EL_MSG_IS_READ_ONLY:
      return 1;
    case M2_EL_MSG_GET_OPT:
	/* d option is not working... */
	/* break out of the switch and let the base class do the rest of the work */
	break;
    case M2_EL_MSG_GET_LIST_BOX:
      ((m2_pcbox_p)(fn_arg->data))->c.y = ((m2_pcbox_p)(fn_arg->data))->p.y; 
      ((m2_pcbox_p)(fn_arg->data))->c.x = ((m2_pcbox_p)(fn_arg->data))->p.x; 
      return 1; /* always visible, visibility is controlled by 'd' option */
    case M2_EL_MSG_GET_LIST_ELEMENT:
     /* reuse from m2elalign.c */
      *((m2_rom_void_p *)(fn_arg->data)) = m2_el_align_get_element(fn_arg);
      return 1;
    case M2_EL_MSG_GET_HEIGHT:
      if ( val >= 2 ) 
        return 0;
      return m2_el_align_get_child_size(fn_arg, 1);
    case M2_EL_MSG_GET_WIDTH:
      if ( val >= 2 ) 
        return 0;
      return m2_el_align_get_child_size(fn_arg, 0);	        
    case M2_EL_MSG_SHOW:
      return 1;
  }
  return m2_el_fnfmt_fn(fn_arg);
}
    

    
    
    
