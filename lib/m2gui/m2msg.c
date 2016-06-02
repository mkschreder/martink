/*

  m2msg.c
  
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

const char *m2_message_text;
const char *m2_message_button1;
m2_button_fnptr m2_message_fn1;
const char *m2_message_button2;
m2_button_fnptr m2_message_fn2;

static void m2_fn_message_b1(m2_el_fnarg_p fnarg) 
{
  if ( m2_message_fn1 != (m2_button_fnptr)NULL )
    m2_message_fn1(fnarg);
}

static void m2_fn_message_b2(m2_el_fnarg_p fnarg) 
{
  if ( m2_message_fn2 != (m2_button_fnptr)NULL )
    m2_message_fn2(fnarg);
}

M2_LABELPTR(m2_message_el_label, "W64f8", &m2_message_text);
M2_BUTTONPTR(m2_message_el_b1, NULL, &m2_message_button1, m2_fn_message_b1);
M2_LIST(m2_message_list) = {&m2_message_el_label, &m2_message_el_b1 };
M2_VLIST(m2_message_el_vlist, NULL, m2_message_list);
M2_ALIGN(m2_message_el_top_align, "W128H128", &m2_message_el_vlist);

void m2_MessageFn(const char *text, const char *button, m2_button_fnptr fn)
{
  m2_message_text = text;
  m2_message_button1 = button;
  m2_message_fn1 = fn;
  m2_SetRoot(&m2_message_el_top_align);
}

M2_BUTTONPTR(m2_message_el_b2, NULL, &m2_message_button2, m2_fn_message_b2);

M2_LIST(m2_message_b2list) = {&m2_message_el_b1, &m2_message_el_b2 };
M2_HLIST(m2_message_el_b2list, NULL, m2_message_b2list);
M2_ALIGN(m2_message_el_button_align_b2, "W64", &m2_message_el_b2list);

M2_LIST(m2_message_listb2) = {&m2_message_el_label, &m2_message_el_button_align_b2 };
M2_VLIST(m2_message_el_vlistb2, NULL, m2_message_listb2);
M2_ALIGN(m2_message_el_top_align_b2, "W64H64", &m2_message_el_vlistb2);


void m2_MessageB2Fn(const char *text, const char *button1, m2_button_fnptr fn1, const char *button2, m2_button_fnptr fn2)
{
  m2_message_text = text;
  m2_message_button1 = button1;
  m2_message_fn1 = fn1;
  m2_message_button2 = button2;
  m2_message_fn2 = fn2;
  m2_SetRoot(&m2_message_el_top_align_b2);
}

