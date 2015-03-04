/*
  m2.c
  
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


  
  Top Level API of the m2tklib.
  Most important: Init and event processing  
  - read keys 
  - pass key values to the event handler
  

  File Structure
    m2.c			Event Processing
      m2eh2bs		2 button simplified handler
      m2eh2bd		2 button handler with data entry mode
  
  
*/

#include "m2.h"

m2_t m2_global_object;



void m2_Init(m2_rom_void_p element, m2_es_fnptr es, m2_eh_fnptr eh, m2_gfx_fnptr gh)
{
  m2_InitM2(&m2_global_object, element, es, eh, gh);
}

void m2_CheckKey(void)
{
  m2_CheckKeyM2(&m2_global_object);
}

/*
  return:
    0: nothing happend
    1:	event has been processed
*/
uint8_t m2_HandleKey(void)
{
  return m2_HandleKeyM2(&m2_global_object);  
}

void m2_Draw(void)
{
  m2_DrawM2(&m2_global_object);
}

m2_rom_void_p m2_FindByXY(uint8_t x, uint8_t y, uint8_t is_change_focus, uint8_t is_send_select)
{
  return m2_FindByXYM2(&m2_global_object, x, y, is_change_focus, is_send_select);
}

void m2_SetFont(uint8_t font_idx, const void *font_ptr)
{
  m2_SetFontM2(&m2_global_object, font_idx, font_ptr);
}

void m2_SetKey(uint8_t key)
{
  m2_PutKeyIntoQueue(&m2_global_object, key);
}

// Get a key and bypass all other m2 procedures. 
// Usually you do not want to call this, use m2_HandleKey() instead 
uint8_t m2_GetKey(void)
{
  return m2_GetKeyFromQueue(&m2_global_object, NULL, NULL);
}

void m2_InitEventSource(void)
{
  m2_SetEventSourceHandlerM2(&m2_global_object, m2_global_object.es);  
}

void m2_SetGraphicsHandler(m2_gfx_fnptr gh)
{
  m2_SetGraphicsHandlerM2(&m2_global_object, gh);
}

void m2_SetHome(m2_rom_void_p element)
{
  m2_SetHomeM2(&m2_global_object, element);
}

void m2_SetHome2(m2_rom_void_p element)
{
  m2_SetHome2M2(&m2_global_object, element);
}

void m2_SetRootChangeCallback(m2_root_change_fnptr cb)
{
  m2_SetRootChangeCallbackM2(&m2_global_object, cb);
}

void m2_SetRoot(m2_rom_void_p element)
{
  m2_SetRootM2(&m2_global_object, element, 0, 0);
}

void m2_SetRootExtended(m2_rom_void_p element, uint8_t next_cnt, uint8_t change_value)
{
  m2_SetRootM2(&m2_global_object, element, next_cnt, change_value);
}

m2_rom_void_p m2_GetRoot(void)
{
  return m2_GetRootM2(&m2_global_object);
}

void m2_Clear(void)
{
  m2_ClearM2(&m2_global_object);
}