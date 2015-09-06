/*

  m2ghu8gutl.c
  
  utility procedures for the u8g graphics handler
  
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

/*
struct m2_u8g_dev_variables_struct
{
  u8g_t *m2_u8g = NULL;
  u8g_uint_t m2_u8g_height_minus_one = 0;
  uint8_t m2_u8g_fg_text_color;
  uint8_t m2_u8g_bg_text_color;
  uint8_t m2_u8g_current_text_color;
  uint8_t m2_u8g_draw_color;

  uint8_t m2_gh_u8g_current_depth;
  uint8_t m2_gh_u8g_invert_at_depth = 255;

  uint8_t m2_gh_u8g_invisible_frame_border_x_size = 1;
  uint8_t m2_gh_u8g_additional_text_border_x_size = 1;
  uint8_t m2_gh_u8g_additional_read_only_border_x_size = 1;

  uint8_t (*m2_gh_u8g_icon_draw)(m2_gfx_arg_p  arg) = m2_gh_dummy;
};
*/

struct m2_u8g_dev_variables_struct m2_u8g_dev_variables =
{
  NULL, 0, 0, 0, 0, 0, 
  0, 255, 1, 1, 1, 
  m2_gh_dummy
};


void m2_SetU8gInvisibleFrameXBorder(uint8_t w)
{
  m2_u8g_dev_variables.m2_gh_u8g_invisible_frame_border_x_size = w;
}

void m2_SetU8gAdditionalTextXBorder(uint8_t w)
{
  m2_u8g_dev_variables.m2_gh_u8g_additional_text_border_x_size = w;
}

/* this affects the readonly messages (M2_GFX_MSG_GET_READONLY_BORDER_WIDTH) and not the readonly flag */
void m2_SetU8gAdditionalReadOnlyXBorder(uint8_t w)
{
  m2_u8g_dev_variables.m2_gh_u8g_additional_read_only_border_x_size = w;
}


/* update local information */
static void m2_u8g_update(void)
{
  m2_u8g_dev_variables.m2_u8g_height_minus_one = u8g_GetHeight(m2_u8g_dev_variables.m2_u8g);
  m2_u8g_dev_variables.m2_u8g_height_minus_one--;
  
  /* force lower left edge of a text as reference */
  u8g_SetFontPosBottom(m2_u8g_dev_variables.m2_u8g);

  m2_u8g_dev_variables.m2_u8g_fg_text_color = u8g_GetDefaultForegroundColor(m2_u8g_dev_variables.m2_u8g);
  m2_u8g_dev_variables.m2_u8g_bg_text_color = u8g_GetDefaultBackgroundColor(m2_u8g_dev_variables.m2_u8g);
  m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
  m2_u8g_dev_variables.m2_u8g_draw_color = u8g_GetDefaultForegroundColor(m2_u8g_dev_variables.m2_u8g);
}


void m2_SetU8g(u8g_t *u8g, uint8_t (*gh_icon_draw)(m2_gfx_arg_p  arg))
{
  m2_u8g_dev_variables.m2_u8g = u8g;
  m2_u8g_dev_variables.m2_gh_u8g_icon_draw = gh_icon_draw;
  m2_u8g_update();
}

void m2_u8g_draw_frame(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h)
{
  u8g_uint_t y;
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
  y -= y0;
  y -= h;
  y++;			/* 13 Jan 2013: Issue 95 */
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_draw_color);
  u8g_DrawFrame(m2_u8g_dev_variables.m2_u8g, x0, y, w, h);
  /*
  printf("draw_frame: x=%d y=%d w=%d h=%d\n", x0, y, w, h);
  */
}

void m2_u8g_draw_frame_shadow(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h)
{
  u8g_uint_t y;
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
  y -= y0;
  y -= h;
  y++;			/* 13 Jan 2013: Issue 95 */
  w--;
  h--;
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_draw_color);
  u8g_DrawFrame(m2_u8g_dev_variables.m2_u8g, x0, y, w, h);
  u8g_DrawVLine(m2_u8g_dev_variables.m2_u8g, x0+w, y+1, h);
  u8g_DrawHLine(m2_u8g_dev_variables.m2_u8g, x0+1, y+h, w);
}

/* origin is low left */
void m2_u8g_draw_box(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h)
{
  u8g_uint_t y;
  /* transform y to upper left */
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
  y -= y0;
  y -= h;
  y++;			/* 13 Jan 2013: Issue 95 */
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_draw_color);
  u8g_DrawBox(m2_u8g_dev_variables.m2_u8g, x0, y, w, h);
}



const u8g_fntpgm_uint8_t *m2_gh_u8g_fonts[4];
int8_t m2_gh_u8g_ref_dx[4];
int8_t m2_gh_u8g_ref_num_dx[4];

const u8g_fntpgm_uint8_t *m2_u8g_get_font(uint8_t font)
{
  font &= 3;
  return m2_gh_u8g_fonts[font];
}

static int8_t m2_u8g_get_reference_delta_x(uint8_t font)
{
  font &= 3;
  return m2_gh_u8g_ref_dx[font];
}

static int8_t m2_u8g_get_reference_num_delta_x(uint8_t font)
{
  font &= 3;
  return m2_gh_u8g_ref_num_dx[font];
}


uint8_t m2_gh_u8g_base(m2_gfx_arg_p  arg)
{
  /* Do a safety check here: Abort if m2_SetU8g has not been called */
  if ( m2_u8g_dev_variables.m2_u8g == NULL )
    return 0;


  /* Proceed with normal message processing */
  switch(arg->msg)
  {
    case M2_GFX_MSG_INIT:		
      break;
    case M2_GFX_MSG_START:
      /* check for proper setup */
      
      break;
    case M2_GFX_MSG_END:
      break;
    case M2_GFX_MSG_DRAW_HLINE:
			u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
      //u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_draw_color);
			u8g_DrawHLine(m2_u8g_dev_variables.m2_u8g, arg->x, m2_u8g_dev_variables.m2_u8g_height_minus_one - arg->y, arg->w);
      break;
    case M2_GFX_MSG_DRAW_VLINE:
      u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
       //u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_draw_color);
			u8g_DrawVLine(m2_u8g_dev_variables.m2_u8g, arg->x, m2_u8g_dev_variables.m2_u8g_height_minus_one - arg->y, arg->h);  
      break;
    case M2_GFX_MSG_DRAW_BOX:
      m2_u8g_draw_box(arg->x, arg->y, arg->w, arg->h);
      break;
    case M2_GFX_MSG_DRAW_TEXT:
      {
        u8g_uint_t x = arg->x;
        u8g_uint_t y;
				
				u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
        u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_get_font(arg->font));
        u8g_SetFontPosBottom(m2_u8g_dev_variables.m2_u8g);
              
        if ( (arg->font & 8) != 0 )
        {
          if ( arg->w != 0 )
          {
            x = arg->w;
            x -= u8g_GetStrWidth(m2_u8g_dev_variables.m2_u8g, arg->s);
            x >>= 1;
            x += arg->x;
          }
        }
        y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
        y -= arg->y;
	y++;			/* 13 Jan 2013: Issue 95, problem 2 */
        x -= u8g_GetStrX(m2_u8g_dev_variables.m2_u8g, arg->s);
        //x += m2_gh_u8g_additional_text_border_x_size;
        u8g_DrawStr(m2_u8g_dev_variables.m2_u8g, x, y, arg->s);
        // printf("DRAW_TEXT: x=%d y=%d s=%s\n", x, y, arg->s);
      }
      break;
    case M2_GFX_MSG_DRAW_TEXT_P:
      {
	u8g_uint_t x = arg->x;
	u8g_uint_t y;
        
        u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
        u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_get_font(arg->font));
        u8g_SetFontPosBottom(m2_u8g_dev_variables.m2_u8g);
        
	      if ( (arg->font & 8) != 0 )
        {
	        if ( arg->w != 0 )
	        {
	          x = arg->w;
	          x -= u8g_GetStrWidthP(m2_u8g_dev_variables.m2_u8g, (const u8g_pgm_uint8_t *)arg->s);
	          x >>= 1;
	          x += arg->x;
	        }
        }
        y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
      	y -= arg->y;
        x -= u8g_GetStrXP(m2_u8g_dev_variables.m2_u8g, (const u8g_pgm_uint8_t *)arg->s);
        //x += m2_gh_u8g_additional_text_border_x_size;
	y++;			/* 13 Jan 2013: Issue 95, problem 2 */
      	u8g_DrawStrP(m2_u8g_dev_variables.m2_u8g, x, y, (const u8g_pgm_uint8_t *)arg->s);
      }
      break;
    case M2_GFX_MSG_DRAW_NORMAL_DATA_ENTRY:
      m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
      //u8g_DrawHLine(m2_u8g, arg->x, m2_u8g_height_minus_one - arg->y - 1, arg->w);
      u8g_DrawHLine(m2_u8g_dev_variables.m2_u8g, arg->x, m2_u8g_dev_variables.m2_u8g_height_minus_one - arg->y , arg->w); 	/* 13 Jan 2013: Issue 95, problem 2 */
      break;
    case M2_GFX_MSG_DRAW_SMALL_DATA_ENTRY:
      m2_u8g_dev_variables.m2_u8g_current_text_color = m2_u8g_dev_variables.m2_u8g_fg_text_color;
      // u8g_DrawHLine(m2_u8g, arg->x, m2_u8g_height_minus_one - arg->y - 1, arg->w);
      u8g_DrawHLine(m2_u8g_dev_variables.m2_u8g, arg->x, m2_u8g_dev_variables.m2_u8g_height_minus_one - arg->y , arg->w); 	/* 13 Jan 2013: Issue 95, problem 2 */
      break;
    case M2_GFX_MSG_SET_FONT:
      {
	      uint8_t idx;
	      idx = arg->font;
	      idx &=3;
	      m2_gh_u8g_fonts[idx] = (const u8g_fntpgm_uint8_t *)(arg->s);
        u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_gh_u8g_fonts[idx]);
        m2_gh_u8g_ref_dx[idx] = u8g_GetGlyphDeltaX(m2_u8g_dev_variables.m2_u8g, 'm');
	m2_gh_u8g_ref_num_dx[idx] = u8g_GetGlyphDeltaX(m2_u8g_dev_variables.m2_u8g, '0');
	if ( m2_gh_u8g_ref_dx[idx] < m2_gh_u8g_ref_num_dx[idx] )
		m2_gh_u8g_ref_dx[idx] = m2_gh_u8g_ref_num_dx[idx];
      }
      return 0;
    case M2_GFX_MSG_GET_TEXT_WIDTH:
      u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_get_font(arg->font));
      return u8g_GetStrPixelWidth(m2_u8g_dev_variables.m2_u8g, arg->s);
    case M2_GFX_MSG_GET_TEXT_WIDTH_P:
      u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_get_font(arg->font));
      return u8g_GetStrPixelWidthP(m2_u8g_dev_variables.m2_u8g, (const u8g_pgm_uint8_t *)arg->s);
    case M2_GFX_MSG_GET_NUM_CHAR_WIDTH:
      return m2_u8g_get_reference_num_delta_x(arg->font);
    case M2_GFX_MSG_GET_CHAR_WIDTH:
      return m2_u8g_get_reference_delta_x(arg->font);
    case M2_GFX_MSG_GET_CHAR_HEIGHT:
      u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_get_font(arg->font));
      return u8g_GetFontLineSpacing(m2_u8g_dev_variables.m2_u8g);
      
    case M2_GFX_MSG_GET_DISPLAY_WIDTH:
      /* printf("display width = %d\n", (int)u8g_GetWidth(m2_u8g)); */
      return u8g_GetWidth(m2_u8g_dev_variables.m2_u8g);
    case M2_GFX_MSG_GET_DISPLAY_HEIGHT:
      /* printf("display height = %d\n", (int)u8g_GetHeight(m2_u8g)); */
      return u8g_GetHeight(m2_u8g_dev_variables.m2_u8g);
    case M2_GFX_MSG_DRAW_VERTICAL_SCROLL_BAR:
      /* scroll bar: "total" total number of items */
      /* scroll bar: "top" topmost item (first visible item) 0 .. total-visible*/
      /* scroll bar: "visible" number of visible items 0 .. total-1 */

      m2_u8g_draw_frame(arg->x, arg->y, arg->w, arg->h);
      {
      	uint16_t h, y;
       	h = m2_utl_sb_get_slider_height(arg->h-2, arg->total, arg->visible);
      	y = m2_utl_sb_get_slider_position(arg->h-2, h, arg->total, arg->visible, arg->top); 	
      	m2_u8g_draw_box(arg->x+1, arg->y+arg->h-1-h-y, arg->w-2, h);
      }
      return 1;
    
    case M2_GFX_MSG_GET_SMALL_BORDER_HEIGHT:
      return 0;
    case M2_GFX_MSG_GET_SMALL_BORDER_WIDTH:
      return 0;
    case M2_GFX_MSG_GET_SMALL_BORDER_X_OFFSET:
      return 0;
    case M2_GFX_MSG_GET_SMALL_BORDER_Y_OFFSET:
      return 0;
      
    case M2_GFX_MSG_GET_READONLY_BORDER_WIDTH:
      return 2*m2_u8g_dev_variables.m2_gh_u8g_additional_read_only_border_x_size;
    case M2_GFX_MSG_GET_READONLY_BORDER_X_OFFSET:
      return m2_u8g_dev_variables.m2_gh_u8g_additional_read_only_border_x_size;
    case M2_GFX_MSG_GET_READONLY_BORDER_HEIGHT:
      return 0;
    case M2_GFX_MSG_GET_READONLY_BORDER_Y_OFFSET:
      return 0;
    
    /*
      The following two messages are used to track the current depth within the gfx driver.
      As soon as the depth changes, the current depth is assigned to the variable m2_gh_u8g_current_depth
      At any depth the gfx drive can decide to invert the font, marked by m2_gh_u8g_invert_at_depth.
      m2_gh_u8g_invert_at_depth must be set in the driver, however it is reset by LEVEL_UP
    */
    case M2_GFX_MSG_LEVEL_DOWN:
      // printf("down %d\n", arg->top);
      m2_u8g_dev_variables.m2_gh_u8g_current_depth = arg->top;
      break;
    case M2_GFX_MSG_LEVEL_NEXT:
      // printf("next %d ", arg->top);
    case M2_GFX_MSG_LEVEL_UP:
      // printf("up %d\n", arg->top);
      m2_u8g_dev_variables.m2_gh_u8g_current_depth = arg->top;
      if ( m2_u8g_dev_variables.m2_gh_u8g_invert_at_depth >= m2_u8g_dev_variables.m2_gh_u8g_current_depth )
      {
        // printf("invert remove %d\n", m2_gh_u8g_invert_at_depth);
        m2_u8g_dev_variables.m2_gh_u8g_invert_at_depth = 255;
        //m2_u8g_current_text_color = m2_u8g_fg_text_color;
      }
      break;
    case M2_GFX_MSG_DRAW_XBM_P:
      {
	u8g_uint_t x = arg->x;
	u8g_uint_t y;        
        u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
        y = m2_u8g_dev_variables.m2_u8g_height_minus_one;
      	y -= arg->y;
	y -= arg->h;
	y++;			/* 13 Jan 2013: Issue 95 */
      	u8g_DrawXBMP(m2_u8g_dev_variables.m2_u8g, x, y, arg->w, arg->h, (const uint8_t *)arg->s);
      }
  }

  return m2_u8g_dev_variables.m2_gh_u8g_icon_draw(arg);
}


