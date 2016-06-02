/*
  m2obj.c
  
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


  
  Simplified Top Level API of the m2tklib.
  Most important: Init and event processing  
  - read keys 
  - pass key values to the event handler
  
  
  
*/

#include "m2.h"

void m2_SetEventSourceHandlerM2(m2_p m2, m2_es_fnptr es)
{
  if ( es != NULL )
  {
    m2->es = es;
    m2->es(m2, M2_ES_MSG_INIT);
  }
}

void m2_SetGraphicsHandlerM2(m2_p m2, m2_gfx_fnptr gh)
{
  if ( gh != NULL )
  {
    m2->gh = gh;
    m2_gfx_init(gh);
    m2_PutKeyIntoQueue(m2, M2_KEY_REFRESH);
  }
}

void m2_SetHomeM2(m2_p m2, m2_rom_void_p element)
{
  m2->home = element;
}

void m2_SetHome2M2(m2_p m2, m2_rom_void_p element)
{
  m2->home2 = element;
}

void m2_SetRootChangeCallbackM2(m2_p m2, m2_root_change_fnptr cb)
{
  m2->root_change_callback = cb;
}

static void m2_root_change_default_cb(m2_rom_void_p new_root, m2_rom_void_p old_root, uint8_t change_value)
{
	(void)new_root; 
	(void)old_root; 
	(void)change_value; 
}



/*
  element: 	the root element
  es:			event source handler, can be NULL
  eh:			event handler to process the source events
  gh:			graphics handler
*/
void m2_InitM2(m2_p m2, m2_rom_void_p element, m2_es_fnptr es, m2_eh_fnptr eh, m2_gfx_fnptr gh)
{
  m2->arg1 = 0;
  m2->arg2 = 0;
  m2->is_frame_draw_at_end = 0;
  m2->key_queue_len = 0;
  m2->key_queue_pos = 0;
  m2->is_last_key_touch_screen_press = 0;
  m2->element_focus = NULL;
  m2->eh = eh;
  m2->gh = gh;
  m2->root_change_callback = m2_root_change_default_cb;  /* called in m2navinit.c */
  m2_gfx_init(gh);
  m2->is_frame_draw_at_end = m2_gfx_is_frame_draw_at_end();
  //m2->forced_key = M2_KEY_REFRESH;
  m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_PRESS;
  m2->home = element;
  m2->home2 = element;
  /* m2_SetHomeM2(m2, element); */
  m2_nav_init(m2_get_nav(m2),  element);
  if ( m2_GetRootM2(m2) != &m2_null_element )	/* only refresh screen for none empty menues, issue 97 */
    m2_PutKeyIntoQueue(m2, M2_KEY_REFRESH);
  m2_get_nav(m2)->root_change_value = 0;
  m2_SetEventSourceHandlerM2(m2, es);
}

void m2_SetEventSourceArgsM2(m2_p m2, uint8_t arg1, uint8_t arg2)
{
  m2->arg1 = arg1;
  m2->arg2 = arg2;
}

void m2_CheckKeyM2(m2_p m2)
{
  uint8_t key;
  /* step 1: get raw key */
  
  /* check if a key should be forced */
  /* obsolete
  key = m2->forced_key;
  if ( key != M2_KEY_NONE )
  {
    m2->forced_key = M2_KEY_NONE;
    m2_PutKeyIntoQueue(m2, key);
  }
  else
  */
  {
    /* request key information from the event source */
    if ( m2->es != NULL )
    {
      key = m2->es(m2, M2_ES_MSG_GET_KEY);
      /*
      if ( key == 0 )
	printf(".");
      else
	printf("key: %d   is_last_key_touch_screen_press:%d\n", key, m2->is_last_key_touch_screen_press);
      */
      
      if ( key == M2_KEY_EVENT(M2_KEY_TOUCH_PRESS) )
      {
	/* event without debounce */
	m2->is_last_key_touch_screen_press = 1;
      }
      /* the case where key == M2_KEY_TOUCH_PRESS is handled inside the deboucne algorithm */      
      if ( m2->is_last_key_touch_screen_press != 0 && key == M2_KEY_NONE )
      {
	m2->is_last_key_touch_screen_press = 0;
	key = M2_KEY_EVENT(M2_KEY_TOUCH_RELEASE);
      }
      
#ifdef OBSOLTE
      
      /* this code automatically generates the M2_KEY_TOUCH_RELEASE message once */
      if ( key == M2_KEY_TOUCH_PRESS || key == M2_KEY_EVENT(M2_KEY_TOUCH_PRESS) )
      {
	m2->is_last_key_touch_screen_press = 1;
	//puts("key == M2_KEY_TOUCH_PRESS");
      }
      else if ( key == M2_KEY_TOUCH_RELEASE || key == M2_KEY_EVENT(M2_KEY_TOUCH_RELEASE) )
      {
	//puts("key == M2_KEY_TOUCH_RELEASE || key == M2_KEY_EVENT(M2_KEY_TOUCH_RELEASE)");
	m2->is_last_key_touch_screen_press = 0;
	key = M2_KEY_EVENT(M2_KEY_TOUCH_RELEASE);
      }
      else if ( m2->is_last_key_touch_screen_press != 0 && key == M2_KEY_NONE )
      {
	//puts("m2->is_last_key_touch_screen_press != 0 && key == M2_KEY_NONE");
	m2->is_last_key_touch_screen_press = 0;
	key = M2_KEY_EVENT(M2_KEY_TOUCH_RELEASE);
      }
#endif
	
      /* store the key in the queue */
      m2_SetDetectedKey(m2, key, m2->arg1, m2->arg2);
    }
    else
      key = M2_KEY_NONE;
  }
}


static uint8_t m2_check_and_assign_new_root(m2_p m2) M2_NOINLINE;
static uint8_t m2_check_and_assign_new_root(m2_p m2)
{
  m2_nav_p nav = m2_get_nav(m2);
  m2_rom_void_p old_element = nav->element_list[0];
  m2_rom_void_p new_element = nav->new_root_element;
  if ( m2_nav_check_and_assign_new_root(nav) != 0 ) 		/* m2navroot.c */
  {
    m2->root_change_callback(new_element, old_element, nav->root_change_value);
    nav->root_change_value = 0;
    return 1;	/* break and let redraw */
  }
  return 0;
}

/*
  return:
    0: nothing happend
    1:	event has been processed
*/
uint8_t m2_HandleKeyM2(m2_p m2)
{
  uint8_t is_redraw_required = 0;
  uint8_t key, arg1, arg2;
  
  arg1 = 0;
  arg2 = 0;
  
  if ( m2_check_and_assign_new_root(m2) != 0 ) 		/* m2navroot.c */
    return 1;	/* break and let redraw */
  
  if ( m2_GetRootM2(m2) == &m2_null_element )
    return 0;
  
  for(;;)
  {
    /* check if the root node has been changed */
    if ( m2_check_and_assign_new_root(m2) != 0 )		/* m2navroot.c */
      return 1;	/* break and let redraw */

    key = m2_GetKeyFromQueue(m2, &arg1, &arg2);
    
    /* if there are no more keys, break out of the loop */
    if ( key == M2_KEY_NONE )
      break;
    
    /* otherwise, process the key event */    
    if ( key == M2_KEY_HOME ) /* aways process the HOME key */
    {
      m2_SetRootM2(m2, m2->home, 0, 0);
    }
    else if ( key == M2_KEY_HOME2 ) /* aways process the HOME2 key */
    {
      m2_SetRootM2(m2, m2->home2, 0, 0);
    }
    /* note, that key numbers are equal to message numbers */
    else if ( m2->eh != NULL )
    {
      /* handle all keys except HOME key */
      m2->eh(m2, key, arg1, arg2);
    }
    
    is_redraw_required = 1;
  }
  
  return is_redraw_required;
}

void m2_SetFontM2(m2_p m2, uint8_t font_idx, const void *font_ptr)
{
  m2_gfx_set_font(m2->gh, font_idx, font_ptr);
  m2_PutKeyIntoQueue(m2, M2_KEY_REFRESH);
}

void m2_SetRootM2(m2_p m2, m2_rom_void_p element, uint8_t next_cnt, uint8_t change_value)
{
  m2_nav_set_root(m2_get_nav(m2),  element, next_cnt, change_value);
}

m2_rom_void_p m2_GetRootM2(m2_p m2)
{
  return m2_get_nav(m2)->element_list[0];
}

void m2_ClearM2(m2_p m2)
{
  m2_SetRootM2(m2, NULL, 0, 0);
}
