/*

  m2ghu8gffs.c
  
  m2tklib = Mini Interative Interface Toolkit Library
  
  Copyright (C) 2012  olikraus@gmail.com

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

#include "../m2.h"
#include "m2ghu8g.h"

#define BG_COLOR (0x0e0 | 0x010 | 0x000)

uint8_t m2_u8g_highlight_shadow_color = 0x0c0 | 0x000 | 0x000;
uint8_t m2_u8g_highlight_frame_color =  0x0e0 | 0x000 | 0x000;
uint8_t m2_u8g_highlight_bg_color =  0x0e0 | 0x010 | 0x000;

uint8_t m2_u8g_highlight_focus_shadow_color = 0x080 | 0x02;
uint8_t m2_u8g_highlight_focus_frame_color =  0x080 | 0x27;
uint8_t m2_u8g_highlight_focus_bg_color =  0x080 | 0x37;

uint8_t m2_u8g_normal_focus_shadow_color = 0x080 | 0x02;
uint8_t m2_u8g_normal_focus_frame_color =  0x080 | 0x27;
uint8_t m2_u8g_normal_focus_bg_color =  0x080 | 0x37;

uint8_t m2_u8g_small_focus_bg_color = 0x080 | 0x37;		
uint8_t m2_u8g_exit_data_entry_color = 0x080 | 0x37;		

uint8_t m2_u8g_background_color = BG_COLOR;

static void m2_u8g_draw_color_frame_shadow(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t frame_color, uint8_t shadow_color)
{
  u8g_uint_t y;
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
  y -= y0;
  y -= h;
  y++;			/* 13 Jan 2013: Issue 95 */
  w--;
  h--;
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, frame_color);
  u8g_DrawFrame(m2_u8g_dev_variables.m2_u8g, x0, y, w, h);
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, shadow_color);
  u8g_DrawVLine(m2_u8g_dev_variables.m2_u8g, x0+w, y+1, h);
  u8g_DrawHLine(m2_u8g_dev_variables.m2_u8g, x0+1, y+h, w);
}

/* origin is low left */
static void m2_u8g_draw_color_box(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, uint8_t color)
{
  u8g_uint_t y;
  /* transform y to upper left */
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
  y -= y0;
  y -= h;
  y++;			/* 13 Jan 2013: Issue 95 */
  //u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_draw_color);
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, color);
  u8g_DrawBox(m2_u8g_dev_variables.m2_u8g, x0, y, w, h);
}



/*
  color version
  focus cursor: frame with shadow
  highlight (buttom) style: frame with shadow
  highlight + focus style: box + frame with shadow
*/

uint8_t m2_gh_u8g_cffs(m2_gfx_arg_p  arg)
{
  switch(arg->msg)
  {
    case M2_GFX_MSG_DRAW_NORMAL_NO_FOCUS:
      if ( (arg->font & 4) != 0 )
      {
        /* highlight flag is set, draw frame with shadow */
        m2_u8g_draw_color_box(arg->x+1, arg->y+1, arg->w-2, arg->h-2, m2_u8g_highlight_bg_color);
      	m2_u8g_draw_color_frame_shadow(arg->x+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->y, arg->w-2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->h, m2_u8g_highlight_frame_color, m2_u8g_highlight_shadow_color);
      }

      m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
      // if ( m2_gh_u8g_invert_at_depth < m2_gh_u8g_current_depth )
      // {
      //   m2_u8g_current_text_color = m2_u8g_bg_text_color;
      // }
      break;
    case M2_GFX_MSG_DRAW_NORMAL_PARENT_FOCUS:
      break;
    case M2_GFX_MSG_DRAW_NORMAL_FOCUS:
      if ( (arg->font & 4) != 0 )
      {
        /* 
            highlight version
            draw frame with shadow together with a filled box 
        */
	
        //m2_u8g_current_text_color = m2_u8g_bg_text_color;
        m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
        m2_u8g_draw_color_box(arg->x+1, arg->y+1, arg->w-2, arg->h-2, m2_u8g_highlight_focus_bg_color);
      	m2_u8g_draw_color_frame_shadow(arg->x+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->y, arg->w-2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->h, m2_u8g_highlight_focus_frame_color, m2_u8g_highlight_focus_shadow_color);
        // m2_gh_u8g_invert_at_depth = m2_gh_u8g_current_depth;
      }
      else
      {
        /* 
            normal version
            draw only the frame with shadow
        */
        m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
        m2_u8g_draw_color_box(arg->x+1, arg->y+1, arg->w-2, arg->h-2, m2_u8g_normal_focus_bg_color);
      	m2_u8g_draw_color_frame_shadow(arg->x+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->y, arg->w-2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size, arg->h, m2_u8g_normal_focus_frame_color, m2_u8g_normal_focus_shadow_color);
      }
      // printf("invert %d, width %d x:%d y:%d\n", m2_gh_u8g_invert_at_depth, arg->w, arg->x, arg->y);
      break;
    case M2_GFX_MSG_DRAW_SMALL_FOCUS:
	  /*
      m2_u8g_current_text_color = m2_u8g_bg_text_color;
      m2_u8g_draw_color_box(arg->x, arg->y, arg->w, arg->h, m2_u8g_fg_text_color);
      */

      m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
      m2_u8g_draw_color_box(arg->x, arg->y, arg->w, arg->h, m2_u8g_small_focus_bg_color);
      break;
    case M2_GFX_MSG_DRAW_GO_UP:
      /* does not work because of missing xor...
         also: can not be fixed with FRAME_DRAW_AT_END
       m2_u8g_current_text_color = m2_u8g_bg_text_color; */
      m2_u8g_draw_color_box(arg->x, arg->y, arg->w, arg->h/2, m2_u8g_exit_data_entry_color);
      break;
    case M2_GFX_MSG_GET_NORMAL_BORDER_HEIGHT:
      return 3;
    case M2_GFX_MSG_GET_NORMAL_BORDER_WIDTH:
      return 3+2*m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size+2*m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size;
    case M2_GFX_MSG_GET_NORMAL_BORDER_X_OFFSET:
      return 1+m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size+m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size;
    case M2_GFX_MSG_GET_NORMAL_BORDER_Y_OFFSET:
      return 2;
    case M2_GFX_MSG_GET_LIST_OVERLAP_HEIGHT:
      return 0;
    case M2_GFX_MSG_GET_LIST_OVERLAP_WIDTH:
      return 0;
    case M2_GFX_MSG_IS_FRAME_DRAW_AT_END:
      return 0; /* focus (highlight) is drawn first, then the text string */
    case M2_GFX_MSG_START:
      u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_background_color);
      u8g_DrawBox(m2_u8g_dev_variables.m2_u8g, 0, 0, u8g_GetWidth(m2_u8g_dev_variables.m2_u8g), u8g_GetHeight(m2_u8g_dev_variables.m2_u8g));
      break;
  }

  return m2_gh_u8g_base(arg);
}
