
/*

  m2fnarg.c
  
  procedures to call element procedures

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

  Code size status: 
    Proccedure attributes: Optimization completed
    Arithmetic optimization: Done

*/

#include <stddef.h>
#include "m2.h"



/*=========================================================================*/
/* helper functions */
static void _m2_el_fnarg_zero(m2_el_fnarg_t *arg) M2_NOINLINE;
static void _m2_el_fnarg_zero(m2_el_fnarg_t *arg)
{
  arg->element = NULL;
  arg->arg = 0;
  arg->data = NULL;  
  arg->nav = NULL;
}



/*=========================================================================*/

/*
  Note: fn_arg functions can not be used recursivly.
  This means that the following functions must not be called with a field (FN) function itself.
*/
static m2_el_fnarg_t m2_fn_arg;
  
  
void m2_fn_arg_clear(void)
{
  _m2_el_fnarg_zero(&m2_fn_arg);
}

void m2_fn_arg_set_element(m2_rom_void_p element)
{
  m2_fn_arg.element = element;
}

m2_rom_void_p m2_fn_arg_get_element(void)
{
  return m2_fn_arg.element;
}

void m2_fn_arg_set_arg_data(uint8_t arg, void *data)
{
  m2_fn_arg.arg = arg;
  m2_fn_arg.data = data;  
}

void m2_fn_arg_set_nav(m2_nav_p nav)
{
  m2_fn_arg.nav = nav;
}

uint8_t m2_fn_arg_call(uint8_t msg)
{
  m2_fn_arg.msg = msg;
  return m2_rom_get_el_fnptr(m2_fn_arg.element)(&m2_fn_arg);
  // return ((m2_el_fnfmt_p)m2_fn_arg.element)->fn(&m2_fn_arg);
}


/*=========================================================================*/
/* the following function CAN be used recursive */

static uint8_t m2_fn_get_wh(m2_rom_void_p element, uint8_t msg) M2_NOINLINE;

static uint8_t m2_fn_get_wh(m2_rom_void_p element, uint8_t msg)
{
  m2_el_fnarg_t arg;
  arg.msg = msg;
  arg.element = element;
  arg.nav = NULL;
  return m2_rom_get_el_fnptr(arg.element)(&arg);
  //return ((m2_el_fnfmt_p)arg.element)->fn(&arg);
}

uint8_t m2_fn_get_width(m2_rom_void_p element)
{
  return m2_fn_get_wh(element, M2_EL_MSG_GET_WIDTH);
}

uint8_t m2_fn_get_height(m2_rom_void_p element)
{
  return m2_fn_get_wh(element, M2_EL_MSG_GET_HEIGHT);
}

