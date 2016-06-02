/*

  m2key.c

  key debounce and queue

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

#define M2_DEBOUNCE_CNT 3

uint8_t m2_GetKeyFromQueue(m2_p m2, uint8_t *arg1, uint8_t *arg2)
{
  uint8_t key;
  if ( m2->key_queue_len == 0 )
    return M2_KEY_NONE;
  
  {  
#ifdef __AVR__
    uint8_t status_reg = SREG;
    cli();
#endif

    key = m2->key_queue_array[m2->key_queue_pos];
    if ( arg1 != NULL )
      *arg1 = m2->key_queue_arg1[m2->key_queue_pos];
    if ( arg2 != NULL )
      *arg2 = m2->key_queue_arg2[m2->key_queue_pos];    
    
    m2->key_queue_pos++;
    m2->key_queue_pos &= (M2_KEY_QUEUE_LEN-1);
    m2->key_queue_len--;
    
#ifdef __AVR__
    SREG = status_reg;
#endif
  }
  return key;
}

void m2_PutKeyIntoQueueWithArgs(m2_p m2, uint8_t key_code, uint8_t arg1, uint8_t arg2)
{
  uint8_t pos;
  
  if ( key_code == M2_KEY_NONE )
    return;

  {  
#ifdef __AVR__
    uint8_t status_reg = SREG;
    cli();
#endif

    pos = m2->key_queue_pos;
    pos += m2->key_queue_len;
    pos &= (M2_KEY_QUEUE_LEN-1);
    m2->key_queue_array[pos] = key_code;
    m2->key_queue_arg1[pos] = arg1;
    m2->key_queue_arg2[pos] = arg2;
    
    if ( m2->key_queue_len >= M2_KEY_QUEUE_LEN ) 
    {
      m2->key_queue_pos++;
      /* 2013-10-27 added the following two lines */
      if ( m2->key_queue_pos >= M2_KEY_QUEUE_LEN )
        m2->key_queue_pos = 0;
    }
    else
    {
      m2->key_queue_len++;
    }
    
#ifdef __AVR__
    SREG = status_reg;
#endif
  }
}

void m2_PutKeyIntoQueue(m2_p m2, uint8_t key_code)
{
  m2_PutKeyIntoQueueWithArgs(m2, key_code, 0, 0);
}


/*
  debounce key and put key into queue
*/
void m2_SetDetectedKey(m2_p m2, uint8_t key_code, uint8_t arg1, uint8_t arg2)
{
    
  /* if event source has set the EVENT bit, then directly pass this value to the queue */
  
  if ( M2_IS_KEY_EVENT(key_code) )
  {
    key_code &= ~M2_KEY_EVENT_MASK;
    m2_PutKeyIntoQueueWithArgs(m2, key_code, arg1, arg2);
    return;
  }
  
  /* debounce the value */
  /*
    state machine
    Input: key_code
  
    A (reset): no keys detected
	if key_code == NONE, stay in A
	if key_code != NONE, then: reset counter & detected_key_code = key_code & goto B
  
    B: button down wait
	if detected_key_code != key_code goto A
	if counter == 0 then BUTTON PRESSED goto C
	Else counter--, stay in B
  
    C: button pressed, wait for release
	if key_code == NONE goto, reset counter D
	if detected_key_code == key_code stay in C
	if detected_key_code != key_code goto A
    
    D: wait for button release
	if counter == 0 then BUTTON RELEASE goto A 
	if  key_code == NONE then counter--, stay in D
	if detected_key_code == key_code goto C
	if detected_key_code != key_code goto A
	
  */
  
  switch(m2->debounce_state)
  {
    case M2_DEBOUNCE_STATE_WAIT_FOR_KEY_PRESS:
      if ( key_code != M2_KEY_NONE )
      {
	m2->detected_key_timer = M2_DEBOUNCE_CNT;
	m2->detected_key_code = key_code;
	m2->debounce_state = M2_DEBOUNCE_STATE_PRESS;
      }
      break;
    case M2_DEBOUNCE_STATE_PRESS:
      if ( m2->detected_key_code != key_code )
      {
	m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_PRESS;
      }
      else if ( m2->detected_key_timer == 0 )
      {
	m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_RELEASE;
      }
      else
      {
	m2->detected_key_timer--;
      }
      break;
    case M2_DEBOUNCE_STATE_WAIT_FOR_KEY_RELEASE:
      if ( key_code == M2_KEY_NONE )
      {
	m2->detected_key_timer = M2_DEBOUNCE_CNT;
	m2->debounce_state = M2_DEBOUNCE_STATE_RELEASE;
      }
      else if ( m2->detected_key_code != key_code )
      {
	m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_PRESS;
      }
      else
      {
	/* Special handling for touch screens: Repeat sending the touch press event */
	if ( m2->detected_key_code == M2_KEY_TOUCH_PRESS )
	  m2_PutKeyIntoQueueWithArgs(m2, M2_EP_MSG_TOUCH_PRESS, arg1, arg2);
      }
     break;
    case M2_DEBOUNCE_STATE_RELEASE:
      if ( m2->detected_key_code == key_code )
      {
	m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_RELEASE;
      }
      else if ( m2->detected_key_timer == 0 )
      { 
	m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_PRESS;
	if ( m2->detected_key_code == M2_KEY_TOUCH_PRESS )	
	  m2_PutKeyIntoQueueWithArgs(m2, M2_KEY_TOUCH_RELEASE, arg1, arg2);
	else
	  m2_PutKeyIntoQueueWithArgs(m2, m2->detected_key_code, arg1, arg2);
      }
      else if ( key_code != M2_KEY_NONE && m2->detected_key_code != key_code )
      {
	m2->debounce_state = M2_DEBOUNCE_STATE_WAIT_FOR_KEY_PRESS;
      }
      else
      {
	m2->detected_key_timer--;
      }
      break;
  }  
}


