/*

  m2ghu8g.h
  
  graphics handler for u8glib
  
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

#ifndef _M2GHU8G_H
#define _M2GHU8G_H

#include "../m2.h"
#include <lib/u8gfx/u8g.h>

#ifdef __cplusplus
extern "C" {
#endif

struct m2_u8g_dev_variables_struct
{
  u8g_t *m2_u8g;
  u8g_uint_t m2_u8g_height_minus_one;
  uint8_t m2_u8g_fg_text_color;
  uint8_t m2_u8g_bg_text_color;
  uint8_t m2_u8g_current_text_color;
  uint8_t m2_u8g_draw_color;

  uint8_t m2_gh_u8g_current_depth;
  uint8_t m2_gh_u8g_invert_at_depth;

  uint8_t m2_gh_u8g_invisible_frame_border_x_size;
  uint8_t m2_gh_u8g_additional_text_border_x_size;
  uint8_t m2_gh_u8g_additional_read_only_border_x_size;

  uint8_t (*m2_gh_u8g_icon_draw)(m2_gfx_arg_p  arg);

};

extern struct m2_u8g_dev_variables_struct m2_u8g_dev_variables;


/* bg and fg colors for 332 color mode */
extern uint8_t m2_u8g_highlight_shadow_color;
extern uint8_t m2_u8g_highlight_frame_color;
extern uint8_t m2_u8g_highlight_bg_color;

extern uint8_t m2_u8g_highlight_focus_shadow_color;
extern uint8_t m2_u8g_highlight_focus_frame_color;
extern uint8_t m2_u8g_highlight_focus_bg_color;

extern uint8_t m2_u8g_normal_focus_shadow_color;
extern uint8_t m2_u8g_normal_focus_frame_color;
extern uint8_t m2_u8g_normal_focus_bg_color;

extern uint8_t m2_u8g_small_focus_bg_color;		
extern uint8_t m2_u8g_exit_data_entry_color;

extern uint8_t m2_u8g_background_color;


/* graphics handler for the u8glib */
/*

logic:
  1. Part: Focus 
  2. Part: Highlight
  Parts:
    b = Box
    f = Frame
    fs = Shadow Frame

    ffs: 
      focus: shadow frame
      highlight: shadow frame
      highlight+focus: shadow frame with inverted content

uint8_t m2_gh_dogm_bf(m2_gfx_arg_p  arg);
uint8_t m2_gh_dogm_bfs(m2_gfx_arg_p  arg);
uint8_t m2_gh_dogm_fb(m2_gfx_arg_p  arg);
uint8_t m2_gh_dogm_fbs(m2_gfx_arg_p  arg);
uint8_t m2_gh_dogm_ffs(m2_gfx_arg_p  arg);
uint8_t m2_gh_dogxl160(m2_gfx_arg_p arg);
*/
/* graphics handler for the u8glib */

/* 
  fb
  focus: frame
  highlight: box
  highlight+focus: frame+box
*/

uint8_t m2_gh_u8g_fb(m2_gfx_arg_p arg);

/* 
  bf
  focus: box
  highlight: frame
  highlight+focus: frame+box
*/
uint8_t m2_gh_u8g_bf(m2_gfx_arg_p  arg);

/* 
  bfs   --> my favorit style
  focus: box
  highlight: frame with shadow
  highlight+focus: frame with shadow+box
*/
uint8_t m2_gh_u8g_bfs(m2_gfx_arg_p  arg);


/* 
  ffs
  focus: frame with shadow
  highlight: frame with shadow
  highlight+focus: frame with shadow+box
*/
uint8_t m2_gh_u8g_ffs(m2_gfx_arg_p  arg);

/*
  cffs
*/
uint8_t m2_gh_u8g_cffs(m2_gfx_arg_p  arg);


/* icon procedures */

uint8_t m2_u8g_font_icon(m2_gfx_arg_p arg);
// uint8_t m2_u8g_bitmap_icon(m2_gfx_arg_p  arg); /* obsolete, can be replaced by m2_u8g_box_icon */
uint8_t m2_u8g_box_icon(m2_gfx_arg_p arg);

/* icon font setup procedures */

void m2_SetU8gToggleFontIcon(const u8g_fntpgm_uint8_t *font, uint8_t active, uint8_t inactive);
void m2_SetU8gRadioFontIcon(const u8g_fntpgm_uint8_t *font, uint8_t active, uint8_t inactive);

/* connect U8glib and M2tklib */

void m2_SetU8g(u8g_t *u8g, m2_gfx_fnptr gh_icon_draw);          /* expects icon procedure as second argument */

/* Additional style parameters */
void m2_SetU8gInvisibleFrameXBorder(uint8_t w);
void m2_SetU8gAdditionalTextXBorder(uint8_t w);
void m2_SetU8gAdditionalReadOnlyXBorder(uint8_t w);


/* utility procedures for the graphics handler */

void m2_u8g_draw_frame(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h);
void m2_u8g_draw_frame_shadow(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h);
void m2_u8g_draw_box(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h);
//void m2_u8g_draw_icon(uint8_t x, uint8_t y, uint8_t font, uint8_t icon);
//uint8_t m2_u8g_get_icon_height(uint8_t font, uint8_t icon);
//uint8_t m2_u8g_get_icon_width(uint8_t font, uint8_t icon);
const u8g_fntpgm_uint8_t *m2_u8g_get_font(uint8_t font);
uint8_t m2_gh_u8g_base(m2_gfx_arg_p  arg);
 
#ifdef __cplusplus
}
#endif
#endif


