/*

  m2draw.c

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
    Arithmetic optimization: Pending

*/

#include <stddef.h>
#include "m2.h"


/*=========================================================================*/
/* draw procedure */

/* a temporary object which holds the draw cursor */
m2_nav_t m2_draw_current;

/* a reference to the object which describes the focus on the current widget */
m2_nav_p m2_draw_focus;	/* could be replaced by m2_draw_p->nav */

/* 0: frame is drawn before the text or other content */
/* 1: frame is drawn last */
uint8_t m2_is_frame_draw_at_end;

/* reference to the current m2 object */
m2_p m2_draw_p;

/*
  This procedure draws (renders) the current object by sending the M2_EL_MSG_SHOW
  message to the element handler.  
*/

static void m2_draw_visit_node(m2_pos_p box, uint8_t msg)
{
  uint8_t d, arg;
  arg = 0;
  
  /* calculate minimum of current and focus depth */
  d = m2_draw_current.depth;
  
  /* the depth must be 2 or higher. */
  if ( d > 1 )
  {
    /* an element has focus, if the depth is lower or equal to the element with the current focus */
    if ( d <= m2_draw_focus->depth )
    {
      /* valid child positions are stored at depth-2 */
      d--;
      
      /* compare all positions up to the root */
      /* this is: check if the current element is equal to the focus element */
      for(;;)
      {
        d--;
        if ( m2_draw_current.pos[d] != m2_draw_focus->pos[d] )
          break;
          
        /* printf("visit node depth=%d curr_pos=%d | focus_pos=%d\n", d, m2_draw_current.pos[d], m2_draw_focus->pos[d]); */
        if ( d == 0 )
        {
          /* root has been reached and all positions are equal: FOCUS EQUALS ELEMENT */
          arg = 1;
          if ( m2_draw_current.depth == m2_draw_focus->depth )
          {
            arg = 2;
            if ( m2_draw_focus->is_data_entry_active != 0 )
              arg = 3;
          }
          break;
        }
      }
    }
  }
  
  m2_nav_prepare_fn_arg_current_element(&m2_draw_current);
  /* if there is no focus, then check if the touch screen has set the focus ("element_focus") */
  if ( arg == 0 )
    if ( m2_fn_arg_get_element() == m2_draw_p->element_focus )
    {
      if ( m2_draw_current.depth >= 2 )
      {
        if ( m2_draw_current.pos[m2_draw_current.depth-2] == m2_draw_p->pos_element_focus )
        {
          arg = 2;
        }
      }
      else
      {
        arg = 2;
      }
    }
  m2_fn_arg_set_arg_data(arg, box);
  
#ifdef M2_EL_MSG_DBG_SHOW
  if ( msg == M2_EL_MSG_SHOW )
    m2_fn_arg_call(M2_EL_MSG_DBG_SHOW);
#endif
  m2_fn_arg_call(msg);  
}


/*
  This procedure traverses the tree. For each node, the procedure m2_draw_visit_node()
  will be called.
*/

static void m2_draw_sub(m2_pos_p box)
{
  m2_pcbox_t pcbox;
  pcbox.p = *box;
  uint8_t draw_without_focus = 0;
  uint8_t is_focus = 0;
  uint8_t is_auto_skip = 0;
  
  /* the draw depth is limited by the current focus depth */
  /* 2011-02-13: this is probably wrong, it can be deeper because of the 'd' flag */
  /* if ( m2_draw_current.depth < m2_draw_focus->depth ) */
  {
    /* try to go down, if there are children */
    if ( m2_nav_down(&m2_draw_current, 0) != 0 )
    {
      
      /* inform the graphics handler about the new depth */
      m2_gfx_level_down(m2_draw_current.depth);
      
      for(;;)
      {

	/* request child position from the parent */
	m2_nav_prepare_fn_arg_parent_element(&m2_draw_current);
	m2_fn_arg_set_arg_data(m2_draw_current.pos[m2_draw_current.depth-2], &pcbox);
	if ( m2_fn_arg_call(M2_EL_MSG_GET_LIST_BOX) )
	{
	  /* request down flag from the child */
	  m2_fn_arg_set_element(m2_nav_get_current_element(&m2_draw_current));
	  m2_fn_arg_set_arg_data('d', &draw_without_focus);
	  m2_fn_arg_call(M2_EL_MSG_GET_OPT);

	  /* request auto down from child */
	  is_auto_skip = m2_fn_arg_call(M2_EL_MSG_IS_AUTO_SKIP);
	  
	  is_focus = 0;
	  if ( m2_draw_current.depth <= m2_draw_focus->depth )
	    if ( m2_draw_current.pos[m2_draw_current.depth-2]  == m2_draw_focus->pos[m2_draw_current.depth-2] )
	      is_focus = 1;
	  
	    
	  /* draw the child */
	  
	  if ( m2_is_frame_draw_at_end == 0 )
	    m2_draw_visit_node(&(pcbox.c), M2_EL_MSG_SHOW);
	    
	  /* if ( (draw_without_focus == 1) || (m2_draw_current.pos[m2_draw_current.depth-2]  == m2_draw_focus->pos[m2_draw_current.depth-2]) ) */
	  if ( (draw_without_focus == 1) || is_focus != 0 || is_auto_skip != 0 )
	  {
	    m2_draw_sub(&(pcbox.c));
	  }
	  
	  if ( m2_is_frame_draw_at_end != 0 )
	    m2_draw_visit_node(&(pcbox.c), M2_EL_MSG_SHOW);
	} /* M2_EL_MSG_GET_LIST_BOX successfull */
        if ( m2_nav_next(&m2_draw_current) == 0 )
          break;
        /* inform the graphics handler about next element on same level */
        m2_gfx_level_next(m2_draw_current.depth);
        
      } /* for(;;) */
      
      
#ifdef M2_EL_MSG_POST_SHOW
      m2_draw_visit_node(&(pcbox.c), M2_EL_MSG_POST_SHOW);
#endif
      /* inform the graphics handler about leaving the current depth */
      m2_gfx_level_up(m2_draw_current.depth);
            
      /* go up one level */
      m2_nav_up(&m2_draw_current);
    }
  }
}


static void m2_nav_draw(m2_nav_p nav)
{
  m2_pos_t box;
  
  m2_draw_focus = nav;
  
  box.x = 0;
  box.y = 0;
  /*
  box.h = 0;
  box.w = 0;
  */
  if ( nav->depth <= 0 )
    return;
  
  m2_draw_current.element_list[0] = nav->element_list[0];
  m2_draw_current.depth = 1;
  m2_draw_visit_node(&box, M2_EL_MSG_SHOW);
  m2_draw_sub(&box);
#ifdef M2_EL_MSG_POST_SHOW
  m2_draw_visit_node(&box, M2_EL_MSG_POST_SHOW);
#endif
}

void m2_DrawM2(m2_p ep)
{
  m2_draw_p = ep;
  m2_is_frame_draw_at_end = ep->is_frame_draw_at_end;
  m2_gfx_start(ep->gh);
  m2_nav_draw(m2_get_nav(ep));
  m2_gfx_end();
}

/*=========================================================================*/
/* find element which covers x/y */

uint8_t m2_check_x;
uint8_t m2_check_y;

uint16_t m2_check_result_min_wh_product;
uint8_t m2_check_result_pos_element;
m2_rom_void_p m2_check_result_element; 
//uint8_t m2_check_result_k_or_t_flag;
uint8_t m2_check_result_x;
uint8_t m2_check_result_y;
uint8_t m2_check_result_w;
uint8_t m2_check_result_h;
uint8_t m2_check_action_copy_focus;  /* for normal m2 menu operation, this is always true */

static void m2_nav_copy_element_stack(m2_nav_p dest, m2_nav_p src)
{
  uint8_t i;
  for( i = 0; i < M2_DEPTH_MAX; i++)
  {
    dest->pos[i] = src->pos[i];
    dest->element_list[i] = src->element_list[i];
  }
  dest->depth = src->depth;
}

static uint8_t m2_check_fn_get_wh(m2_rom_void_p element, uint8_t msg) M2_NOINLINE;

static uint8_t m2_check_fn_get_wh(m2_rom_void_p element, uint8_t msg)
{
  m2_el_fnarg_t arg;
  arg.msg = msg;
  arg.element = element;
  arg.nav = &m2_draw_current;
  return m2_rom_get_el_fnptr(arg.element)(&arg);
}


static void m2_check_xy(m2_pos_p pos)
{
  m2_rom_void_p element = m2_nav_get_current_element(&m2_draw_current);
  uint8_t w = m2_check_fn_get_wh(element, M2_EL_MSG_GET_WIDTH);
  uint8_t h = m2_check_fn_get_wh(element, M2_EL_MSG_GET_HEIGHT);
  uint8_t x = m2_check_x;
  uint8_t y = m2_check_y;
  uint16_t p;
  uint8_t ro_flag;
  uint8_t t_flag;
  //uint8_t k_flag;
  
  /* box is now pos->x, pos->y, w, h; pos->x,pos->y is lower left edge */
  
  if ( x < pos->x )
    return;
  if ( y < pos->y )
    return;
  
  x -= pos->x;
  y -= pos->y;

  if ( x > w )
    return;
  if ( y > h )
    return;

  /* check for read only flag */
  ro_flag = 0;
  m2_fn_arg_set_element(element);
  m2_fn_arg_set_arg_data('r', &ro_flag);
  m2_fn_arg_call(M2_EL_MSG_GET_OPT);

  /* check for touch screen flag */
  t_flag = 0;
  m2_fn_arg_set_element(element);
  m2_fn_arg_set_arg_data('t', &t_flag);
  m2_fn_arg_call(M2_EL_MSG_GET_OPT);

  /* check for key flag (of touch screens) */
  
  // k_flag = 0;
  // m2_fn_arg_set_element(element);
  // m2_fn_arg_set_arg_data('k', &k_flag);
  // m2_fn_arg_call(M2_EL_MSG_GET_OPT);


  // printf("check: element %p ro %d t %d\n", element, ro_flag, t_flag);

  if ( t_flag == 0 && ro_flag != 0 )	
    return;  /* read only flag is set (but only if this is not a touch screen element */

  if ( t_flag == 0 )
    return;  /* touch screen flag is NOT set, so ignore this element */
  
  p = w*h;
  if ( m2_check_result_min_wh_product >= p )
  {
    m2_check_result_min_wh_product = p;
    //m2_check_result_k_or_t_flag = k_flag | t_flag;
    m2_check_result_element = element;
    m2_check_result_pos_element = m2_draw_current.pos[m2_draw_current.depth-2];
    if ( t_flag != 0 && ro_flag == 0 )				// never copy focus for read only elements 
      if ( m2_check_action_copy_focus != 0 )
      {
        m2_nav_copy_element_stack(m2_draw_focus, &m2_draw_current);
        /* printf("focus: %p : %d\n", &m2_draw_current, m2_draw_focus->pos[m2_draw_focus->depth-2]); */
      }
  }
}


static void m2_find_by_xy_sub(m2_pos_p box)
{
  m2_pcbox_t pcbox;
  pcbox.p = *box;
  uint8_t draw_without_focus = 0;
  uint8_t is_focus = 0;
  uint8_t is_auto_skip = 0;
  
    /* try to go down, if there are children */
    if ( m2_nav_down(&m2_draw_current, 0) != 0 )
    {
      
      for(;;)
      {

	/* request child position from the parent */
	m2_nav_prepare_fn_arg_parent_element(&m2_draw_current);
	m2_fn_arg_set_arg_data(m2_draw_current.pos[m2_draw_current.depth-2], &pcbox);
	
	/* position and size (?) are available in pcbox */
	
	if ( m2_fn_arg_call(M2_EL_MSG_GET_LIST_BOX) )
	{
	  
	  /* request down flag from the child */
	  m2_fn_arg_set_element(m2_nav_get_current_element(&m2_draw_current));
	  m2_fn_arg_set_arg_data('d', &draw_without_focus);
	  m2_fn_arg_call(M2_EL_MSG_GET_OPT);

	  /* request auto down from child */
	  is_auto_skip = m2_fn_arg_call(M2_EL_MSG_IS_AUTO_SKIP);
	  
	  is_focus = 0;
	  if ( m2_draw_current.depth <= m2_draw_focus->depth )
	    if ( m2_draw_current.pos[m2_draw_current.depth-2]  == m2_draw_focus->pos[m2_draw_current.depth-2] )
	      is_focus = 1;

	  m2_check_xy(&(pcbox.c));
	  
	    
	  if ( (draw_without_focus == 1) || is_focus != 0 || is_auto_skip != 0 )
	  {
	    m2_find_by_xy_sub(&(pcbox.c));
	  }
	  
	} /* M2_EL_MSG_GET_LIST_BOX successfull */
        if ( m2_nav_next(&m2_draw_current) == 0 )
          break;
        
      } /* for(;;) */
      
      
      /* go up one level */
      m2_nav_up(&m2_draw_current);
    }
}

static m2_rom_void_p m2_nav_find_by_xy(m2_nav_p nav, uint8_t x, uint8_t y, uint8_t is_change_focus, uint8_t is_send_select)
{
  m2_pos_t box;
  
  m2_draw_focus = nav;

  m2_check_x = x;
  m2_check_y = y;

  m2_check_result_min_wh_product = 0x0ffff;
  // m2_check_result_k_or_t_flag = 0;
  m2_check_result_element = NULL; 
  m2_check_result_pos_element = 255; 
  m2_check_result_x=255;
  m2_check_result_y=255;
  m2_check_result_w=0;
  m2_check_result_h=0;
  m2_check_action_copy_focus = is_change_focus;

  
  box.x = 0;
  box.y = 0;
  
  if ( nav->depth <= 0 )
    return NULL;
  
  m2_draw_current.element_list[0] = nav->element_list[0];
  m2_draw_current.depth = 1;
  m2_check_xy(&box);
  m2_find_by_xy_sub(&box);
  
  if ( m2_check_result_element != NULL )
  {
      /* set the element focus for elements with k format option */
      if ( m2_check_action_copy_focus )
      {
        m2_draw_p->element_focus = m2_check_result_element;
        m2_draw_p->pos_element_focus = m2_check_result_pos_element;
      }
      if ( is_send_select )
      {
        m2_nav_prepare_fn_arg_current_element(nav);
        /* if the focus is not copied (disabled or k-option but no t-option set), overwrite the element from the nav object */
        /* this may lead to some problems, because now, the nav object does not always contain the element */
        m2_fn_arg_set_element(m2_check_result_element);
        m2_fn_arg_set_arg_data(0, (void *)nav);
        m2_fn_arg_call(M2_EL_MSG_SELECT);    
      }
  }
  else
  {
    /* always clear the element focus */
    if ( m2_check_action_copy_focus )
      m2_draw_p->element_focus = NULL;
  }

  
  return m2_check_result_element;
}

m2_rom_void_p m2_FindByXYM2(m2_p ep, uint8_t x, uint8_t y, uint8_t is_change_focus, uint8_t is_send_select)
{
  m2_draw_p = ep;
  return m2_nav_find_by_xy(m2_get_nav(ep), x, y, is_change_focus, is_send_select);
}

