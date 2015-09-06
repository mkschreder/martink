/*

  m2ghu8gicon.c
  
  icon handling for the u8g graphics handler
  
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

/*========================================================================*/
/* Font Icon */

/*
#define M2_ICON_TOGGLE_ACTIVE 1
#define M2_ICON_TOGGLE_INACTIVE 2 
#define M2_ICON_RADIO_ACTIVE 3
#define M2_ICON_RADIO_INACTIVE 4
*/

const u8g_fntpgm_uint8_t *m2_u8g_toggle_icon_font = NULL;
uint8_t m2_u8g_toggle_active = 0;
uint8_t m2_u8g_toggle_inactive = 0;

const u8g_fntpgm_uint8_t *m2_u8g_radio_icon_font = NULL;
uint8_t m2_u8g_radio_active = 0;
uint8_t m2_u8g_radio_inactive = 0;

void m2_SetU8gToggleFontIcon(const u8g_fntpgm_uint8_t *font, uint8_t active, uint8_t inactive)
{
  m2_u8g_toggle_icon_font = font;
  m2_u8g_toggle_active = active;
  m2_u8g_toggle_inactive = inactive;  
}

void m2_SetU8gRadioFontIcon(const u8g_fntpgm_uint8_t *font, uint8_t active, uint8_t inactive)
{
  m2_u8g_radio_icon_font = font;
  m2_u8g_radio_active = active;
  m2_u8g_radio_inactive = inactive;  
}

static void m2_u8g_draw_font_icon(uint8_t x, uint8_t y, uint8_t font, uint8_t icon)
{
	(void)font; 
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one - y;
  y++;			/* 13 Jan 2013: Issue 95 */
  
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
  
  if ( icon == M2_ICON_TOGGLE_ACTIVE || icon == M2_ICON_TOGGLE_INACTIVE )
  {
    if ( m2_u8g_toggle_icon_font == NULL )
      return;
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_toggle_icon_font);
    if ( icon == M2_ICON_TOGGLE_ACTIVE )
      u8g_DrawGlyph(m2_u8g_dev_variables.m2_u8g, x, y, m2_u8g_toggle_active);
    else
      u8g_DrawGlyph(m2_u8g_dev_variables.m2_u8g, x, y, m2_u8g_toggle_inactive);      
  }
  
  if ( icon == M2_ICON_RADIO_ACTIVE || icon == M2_ICON_RADIO_INACTIVE )
  {
    if ( m2_u8g_radio_icon_font == NULL )
      return;
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_radio_icon_font);
    if ( icon == M2_ICON_RADIO_ACTIVE )
      u8g_DrawGlyph(m2_u8g_dev_variables.m2_u8g, x, y, m2_u8g_radio_active);
    else
      u8g_DrawGlyph(m2_u8g_dev_variables.m2_u8g, x, y, m2_u8g_radio_inactive);      
  }
  
}

static uint8_t m2_u8g_get_font_icon_height(uint8_t font, uint8_t icon)
{
	(void)font; 
  if ( icon == M2_ICON_TOGGLE_ACTIVE || icon == M2_ICON_TOGGLE_INACTIVE )
  {
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_toggle_icon_font);
  }
  else if ( icon == M2_ICON_RADIO_ACTIVE || icon == M2_ICON_RADIO_INACTIVE )
  {
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_radio_icon_font);
  }
  else
  {
    return 0;
  }
  return u8g_GetFontAscent(m2_u8g_dev_variables.m2_u8g)-u8g_GetFontDescent(m2_u8g_dev_variables.m2_u8g);
}

static uint8_t m2_u8g_get_font_icon_width(uint8_t font, uint8_t icon)
{
	(void)font; 
  char s[2];
  s[1] = '\0';
  if ( icon == M2_ICON_TOGGLE_ACTIVE || icon == M2_ICON_TOGGLE_INACTIVE )
  {
    s[0] = m2_u8g_toggle_active;
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_toggle_icon_font);
    return u8g_GetStrWidth(m2_u8g_dev_variables.m2_u8g, s);
  }
  
  if ( icon == M2_ICON_RADIO_ACTIVE || icon == M2_ICON_RADIO_INACTIVE )
  {
    s[0] = m2_u8g_radio_active;
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_radio_icon_font);
    return u8g_GetStrWidth(m2_u8g_dev_variables.m2_u8g, s);
  }
  return 0;
}


uint8_t m2_u8g_font_icon(m2_gfx_arg_p  arg)
{
  /* Do a safety check here: Abort if m2_SetU8g has not been called */
  if ( m2_u8g_dev_variables.m2_u8g == NULL )
    return 0;

  /* Proceed with normal message processing */
  switch(arg->msg)
  {
    case M2_GFX_MSG_DRAW_ICON:
      m2_u8g_draw_font_icon(arg->x, arg->y, arg->font, arg->icon);
      break;
    case M2_GFX_MSG_GET_ICON_WIDTH:
      return m2_u8g_get_font_icon_width(arg->font, arg->icon);
    case M2_GFX_MSG_GET_ICON_HEIGHT:
      return m2_u8g_get_font_icon_height(arg->font, arg->icon);
  }
  return m2_gh_dummy(arg);
}


#ifdef OBSOLETE_BITMAP_ICON

/*========================================================================*/
/* Bitmap Icon, fixed size */

/*
  00000000 0x00
  01111110 0x7e
  01000010 0x42
  01000010 0x42
  01000010 0x42
  01000010 0x42
  01111110 0x7e
  00000000 0x00

  00000000 0x00
  01111111 0x7f
  01000011 0x43
  01100110 0x66
  01111110 0x7e
  01011010 0x5a
  01111110 0x7e
  00000000 0x00

dog_pgm_uint8_t m2_dogm_icon0[8] = {0x00, 0x7e, 0x042, 0x042, 0x042, 0x042, 0x07e, 0x00};
dog_pgm_uint8_t m2_dogm_icon1[8] = {0x00, 0x7f, 0x043, 0x066, 0x07e, 0x05a, 0x07e, 0x00};


  11111111 0xff
  10000001 0x81
  10000001 0x81
  10000001 0x81
  10000001 0x81
  10000001 0x81
  10000001 0x81
  11111111 0xff

  11111111 0xff
  10000001 0x81
  10111101 0xbd
  10111101 0xbd
  10111101 0xbd
  10111101 0xbd
  10000001 0x81
  11111111 0xff

*/

u8g_pgm_uint8_t m2_u8g_icon0[8] = {0x0ff, 0x081, 0x081, 0x081, 0x081, 0x081, 0x081, 0x0ff};
u8g_pgm_uint8_t m2_u8g_icon1[8] = {0x0ff, 0x081, 0x0bd, 0x0bd, 0x0bd, 0x0bd, 0x081, 0x0ff};

/*
#define M2_ICON_TOGGLE_ACTIVE 1
#define M2_ICON_TOGGLE_INACTIVE 2 
#define M2_ICON_RADIO_ACTIVE 3
#define M2_ICON_RADIO_INACTIVE 4
*/

void m2_u8g_draw_bitmap_icon(uint8_t x, uint8_t y, uint8_t font, uint8_t icon)
{
  const u8g_pgm_uint8_t *ptr = m2_u8g_icon0;
  if ( icon == M2_ICON_TOGGLE_ACTIVE || icon == M2_ICON_RADIO_ACTIVE )
    ptr = m2_u8g_icon1;
  //dog_SetBitmapP(x,y+7,ptr,8,8);

  u8g_SetColorIndex(m2_u8g, m2_u8g_current_text_color);
  u8g_DrawBitmapP(m2_u8g, x, m2_u8g_dev_variables.m2_u8g_height_minus_one - (y+8), 1, 8, ptr);
}

uint8_t m2_u8g_get_bitmap_icon_height(uint8_t font, uint8_t icon)
{
  return 8;
}

uint8_t m2_u8g_get_bitmap_icon_width(uint8_t font, uint8_t icon)
{
  return 8;
}

uint8_t m2_u8g_bitmap_icon(m2_gfx_arg_p  arg)
{
  /* Do a safety check here: Abort if m2_SetU8g has not been called */
  if ( m2_u8g == NULL )
    return 0;

  /* Proceed with normal message processing */
  switch(arg->msg)
  {
    case M2_GFX_MSG_DRAW_ICON:
      m2_u8g_draw_bitmap_icon(arg->x, arg->y, arg->font, arg->icon);
      break;
    case M2_GFX_MSG_GET_ICON_WIDTH:
      return m2_u8g_get_bitmap_icon_width(arg->font, arg->icon);
    case M2_GFX_MSG_GET_ICON_HEIGHT:
      return m2_u8g_get_bitmap_icon_height(arg->font, arg->icon);
  }
  return m2_gh_dummy(arg);
}
#endif /* OBSOLETE_BITMAP_ICON */


/*========================================================================*/
/* Manually drawn box icon, dynamic size */

static uint8_t m2_u8g_get_box_icon_size(uint8_t font)
{
    int8_t x;
    u8g_SetFont(m2_u8g_dev_variables.m2_u8g, m2_u8g_get_font(font));
    x = u8g_GetFontAscent(m2_u8g_dev_variables.m2_u8g)-u8g_GetFontDescent(m2_u8g_dev_variables.m2_u8g);
    x*=5;
    x >>=3;
    if ( x < 6 )
      x = 6;
    return x;
}

static void m2_u8g_draw_box_icon(uint8_t x, uint8_t y, uint8_t font, uint8_t icon)
{
  uint8_t h;
  h = m2_u8g_get_box_icon_size(font);
  y = m2_u8g_dev_variables.m2_u8g_height_minus_one - y;
  
  u8g_SetColorIndex(m2_u8g_dev_variables.m2_u8g, m2_u8g_dev_variables.m2_u8g_current_text_color);
  u8g_DrawFrame(m2_u8g_dev_variables.m2_u8g, x, y - h, h, h);
  
  if ( icon == M2_ICON_TOGGLE_ACTIVE || icon == M2_ICON_RADIO_ACTIVE )
  {
    h -= 4;
    u8g_DrawBox(m2_u8g_dev_variables.m2_u8g, x + 2, y - h - 2, h, h);
  }
}

uint8_t m2_u8g_box_icon(m2_gfx_arg_p  arg)
{
  /* Do a safety check here: Abort if m2_SetU8g has not been called */
  if ( m2_u8g_dev_variables.m2_u8g == NULL )
    return 0;

  /* Proceed with normal message processing */
  switch(arg->msg)
  {
    case M2_GFX_MSG_DRAW_ICON:
      m2_u8g_draw_box_icon(arg->x, arg->y, arg->font, arg->icon);
      break;
    case M2_GFX_MSG_GET_ICON_WIDTH:
      return m2_u8g_get_box_icon_size(arg->font);
    case M2_GFX_MSG_GET_ICON_HEIGHT:
      return m2_u8g_get_box_icon_size(arg->font);
  }
  return m2_gh_dummy(arg);
}
