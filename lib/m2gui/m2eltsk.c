/*
  
  m2eltsk.c

  A special element for touch-screens. Generates a key event msg (format option 'k').
  
  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2013  olikraus@gmail.com

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


  Available arguments for key:
  M2_KEY_NONE
  M2_KEY_SELECT
  M2_KEY_EXIT
  M2_KEY_NEXT
  M2_KEY_PREV
  M2_KEY_DATA_UP
  M2_KEY_DATA_DOWN
  M2_KEY_HOME

  
*/

#include <stddef.h>
#include "m2.h"

static uint8_t m2_el_tsk_get_key(m2_el_fnarg_p fn_arg)
{
  return m2_rom_get_u8(fn_arg->element, offsetof(m2_el_tsk_t, key));
}


M2_EL_FN_DEF(m2_el_tsk_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_OPT:
	if ( fn_arg->arg == 't' || fn_arg->arg == 'r' )
	{
	  *(uint8_t *)(fn_arg->data) = 1;
	  return 1;
	}
	/* else... break out of the switch and let the base class do the rest of the work */
	break;
    case M2_EL_MSG_SELECT:
    {
      //uint8_t msg = m2_el_fmfmt_opt_get_val_any_default(fn_arg, 'k', 0);
      //uint8_t key_code = m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'k');
      uint8_t key_code = m2_el_tsk_get_key(fn_arg);
      if ( key_code > 0 )
      {
				m2_SetKey( key_code);		/* use global m2 object */
      }
      return 1;
    }
  }
  return m2_el_label_fn(ep, fn_arg);
}

M2_EL_FN_DEF(m2_el_tskp_fn)
{
  switch(fn_arg->msg)
  {
    case M2_EL_MSG_GET_OPT:
	if ( fn_arg->arg == 't' || fn_arg->arg == 'r' )
	{
	  *(uint8_t *)(fn_arg->data) = 1;
	  return 1;
	}
	/* else... break out of the switch and let the base class do the rest of the work */
	break;
    case M2_EL_MSG_SELECT:
    {
      //uint8_t msg = m2_el_fmfmt_opt_get_val_any_default(fn_arg, 'k', 0);
      //uint8_t key_code = m2_el_fmfmt_opt_get_val_zero_default(fn_arg, 'k');
      uint8_t key_code = m2_el_tsk_get_key(fn_arg);
      if ( key_code > 0 )
      {
	m2_SetKey(key_code);		/* use global m2 object */
      }
      return 1;
    }
  }
  return m2_el_labelp_fn(fn_arg);
}
