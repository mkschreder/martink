#pragma once 

#include "gui_impl.h"

M2_EXTERN_XYLIST(top_el_tlsm); 

static struct gui_data model; 

/****************
* CUSTOM CONTROLS
*****************/

struct _m2_el_chan_struct
{
  m2_el_fnfmt_t ff;
  uint32_t *min, *max, *center, *value; 
};
typedef struct _m2_el_chan_struct m2_el_chan_t;
typedef m2_el_chan_t *m2_el_chan_p;

static uint32_t *gui_el_chan_get_min(m2_el_fnarg_p fn_arg)
{
  return (uint32_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_chan_t, min));
}

static uint32_t *gui_el_chan_get_max(m2_el_fnarg_p fn_arg)
{
  return (uint32_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_chan_t, max));
}
static uint32_t *gui_el_chan_get_center(m2_el_fnarg_p fn_arg)
{
  return (uint32_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_chan_t, center));
}
static uint32_t *gui_el_chan_get_value(m2_el_fnarg_p fn_arg)
{
  return (uint32_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_chan_t, value));
}

M2_EL_FN_DEF(m2_el_chval_fn)
{
	//uint8_t font;

	//font = m2_el_fmfmt_get_font(fn_arg);
	
	
	uint16_t h = m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'h' ); 
	uint16_t w = m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'w' ); 
	
	if(w == 0) w = 4; if(h == 0) h = 20; 
			
	switch(fn_arg->msg)
	{
		case M2_EL_MSG_IS_READ_ONLY:
			return 1;
		case M2_EL_MSG_GET_HEIGHT:
			return w; 
		case M2_EL_MSG_GET_WIDTH:
			return h; 
		case M2_EL_MSG_SHOW: {
			if ( m2_is_frame_draw_at_end == 0 )
				m2_el_fnfmt_fn(fn_arg);
				
			m2_pos_p b = (m2_pos_p)(fn_arg->data);
			//m2_gfx_normal_no_focus(0, 0, 1, 1, font); 
			
			uint32_t min = *gui_el_chan_get_min(fn_arg); 
			uint32_t max = *gui_el_chan_get_max(fn_arg); 
			uint32_t center = *gui_el_chan_get_center(fn_arg); 
			uint32_t value = *gui_el_chan_get_value(fn_arg); 
			
			if(value < min) value = min; 
			if(value > max) value = max; 
			
			uint32_t yy, hh; 
			
			if(value < center) { 
				yy = value - min; 
				hh = center - value; 
			} else {
				yy = center - min; 
				hh = value - center; 
			}
			
			// now scale the values to size of the control 
			
			if(w > h) {// horizontal 
				uint32_t vpp = ((max - min) / w); 
				yy = yy / vpp; 
				hh = hh / vpp; 
				
				if(hh == 0) hh = 1; 
			
				m2_gfx_box(b->x + yy, b->y, hh + 1, h); 
				m2_gfx_vline(b->x + (w >> 1), b->y + h, h + 1); 
			} else {// vertical
				uint32_t vpp = ((max - min) / h); 
				yy = yy / vpp; 
				hh = hh / vpp; 
				
				if(hh == 0) hh = 1; 
				
				m2_gfx_box(b->x, b->y + yy, w, hh + 1);
				m2_gfx_hline(b->x, b->y + (h >> 1), w);  
			}
			
			m2_gfx_vline(b->x, b->y + h, h + 1); 
			m2_gfx_vline(b->x + w, b->y + h, h + 1); 
			m2_gfx_hline(b->x, b->y, w); 
			m2_gfx_hline(b->x, b->y + h, w); 
			
			
			if ( m2_is_frame_draw_at_end != 0 )
				m2_el_fnfmt_fn(fn_arg);
		} return 1;    
	}
	return m2_el_fnfmt_fn(fn_arg);
}
#define GUI_CHANNEL(el,fmt, min, max, center, value) M2_EL_CONST m2_el_chan_t el M2_SECTION_PROGMEM = { { m2_el_chval_fn, (fmt) } , (min), (max), (center), (value)}
#define GUI_EXTERN_CHANNEL(el) extern M2_EL_CONST m2_el_chan_t el


struct _m2_el_frame_struct
{
  m2_el_fnfmt_t ff;
};
typedef struct _m2_el_frame_struct m2_el_frame_t;
typedef m2_el_frame_t *m2_el_frame_p;

M2_EL_FN_DEF(m2_el_frame_fn)
{
	//uint8_t font;

	//font = m2_el_fmfmt_get_font(fn_arg);
	switch(fn_arg->msg)
	{
		case M2_EL_MSG_IS_READ_ONLY:
			return 1;
		case M2_EL_MSG_GET_HEIGHT:
			return 1; 
		case M2_EL_MSG_GET_WIDTH:
			return 1; 
		case M2_EL_MSG_SHOW: {
			//m2_pos_p b = (m2_pos_p)(fn_arg->data);
			
			if ( m2_is_frame_draw_at_end == 0 )
				m2_el_fnfmt_fn(fn_arg);
				
			//m2_gfx_normal_no_focus(0, 0, 1, 1, font); 
			//m2_gfx_frame(0, 0, 127, 63); 
			//m2_gfx_box(0, 6, 127, 51); 
			m2_gfx_hline(0, 63, 128); // top
			m2_gfx_hline(0, 56, 128); // top +1 
			m2_gfx_hline(0, 10, 128); // bottom -1
			//m2_gfx_hline(0, 0, 128); // bottom
			
			m2_gfx_vline(0, 63, 54); // left
			m2_gfx_vline(127, 63, 54); // right
			
			if ( m2_is_frame_draw_at_end != 0 )
				m2_el_fnfmt_fn(fn_arg);
		} return 1;    
	}
	return m2_el_fnfmt_fn(fn_arg);
}
#define GUI_FRAME(el,fmt) M2_EL_CONST m2_el_frame_t el M2_SECTION_PROGMEM = { { m2_el_frame_fn, (fmt) } }
#define GUI_EXTERN_FRAME(el) extern M2_EL_CONST m2_el_frame_t el

// GUI SWITCH

struct _m2_el_switch_struct
{
  m2_el_fnfmt_t ff;
  uint8_t *val; 
};
typedef struct _m2_el_switch_struct m2_el_switch_t;
typedef m2_el_switch_t *m2_el_switch_p;

static uint8_t gui_el_switch_get_value(m2_el_fnarg_p fn_arg)
{
  return *((uint8_t *)m2_rom_get_ram_ptr(fn_arg->element, offsetof(m2_el_switch_t, val)));
}

M2_EL_FN_DEF(m2_el_switch_fn)
{
	//uint8_t font;
	uint8_t font;

	font = m2_el_fmfmt_get_font(fn_arg);
	
	uint16_t h = m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'h' ); 
	uint16_t w = m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'w' ); 
	
	if(w == 0) w = 5; if(h == 0) h = 10; 
	
	//font = m2_el_fmfmt_get_font(fn_arg);
	switch(fn_arg->msg)
	{
		case M2_EL_MSG_IS_READ_ONLY:
			return 1;
		case M2_EL_MSG_GET_HEIGHT:
			return m2_gfx_add_readonly_border_height(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
		case M2_EL_MSG_GET_WIDTH:
			return m2_gfx_add_readonly_border_width(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_str_get_str(fn_arg))));
		case M2_EL_MSG_SHOW: {
			m2_pos_p b = (m2_pos_p)(fn_arg->data);
			uint8_t on = gui_el_switch_get_value(fn_arg); 
			//m2_gfx_normal_no_focus(0, 0, 1, 1, font); 
			
			m2_gfx_hline(b->x + 1, b->y + h, w); // top
			m2_gfx_hline(b->x + 1, b->y + 1, w); // bottom
			m2_gfx_vline(b->x + 1, b->y + h, h - 1); // left
			m2_gfx_vline(b->x + w, b->y + h, h - 1); // right
			
			if(on)
				m2_gfx_box(b->x + 1, b->y + (h / 2) + 1, w, h / 2);
			else
				m2_gfx_box(b->x + 1, b->y + 1, w, h / 2);
		} return 1;    
	}
	return m2_el_fnfmt_fn(fn_arg);
}
#define GUI_SWITCH(el,fmt, val) M2_EL_CONST m2_el_switch_t el M2_SECTION_PROGMEM = { { m2_el_switch_fn, (fmt) } , (val)}
#define GUI_EXTERN_SWITCH(el) extern M2_EL_CONST m2_el_switch_t el

static uint32_t chmin = 1000, chmax = 2000, chcenter = 1500; 

//===================
// DIALOG FRAME

/****************
* CALIBRATION DIALOG
*****************/
/*
M2_LABEL(el_cal_title_label, "", "===== calibrate =====");
M2_ALIGN(el_cal_title, "x0y0-1|2W64H64", &el_cal_title_label); 

M2_EXTERN_HLIST(el_mm_ch1); 
M2_EXTERN_HLIST(el_mm_ch2); 
M2_EXTERN_HLIST(el_mm_ch3); 
M2_EXTERN_HLIST(el_mm_ch4); 
M2_EXTERN_HLIST(el_mm_ch5); 
M2_EXTERN_HLIST(el_mm_ch6); 

M2_LIST(el_cal_chi_list) = {
	&el_cal_title, 
		&el_mm_ch1, 
		&el_mm_ch2, 
		&el_mm_ch3, 
		&el_mm_ch4, 
		&el_mm_ch5, 
		&el_mm_ch6
}; 
M2_VLIST(el_cal_chi, "x0y11l5w15", el_cal_chi_list); 

M2_U32NUM(el_cal_ch1_min_value, "a0r1c4", &model.ch[0].min); 
M2_SPACE(el_cal_ch1_space, "W1h1");
M2_U32NUM(el_cal_ch1_max_value, "a0r1c4", &model.ch[0].max); 
M2_LIST(el_cal_ch1_list) = {&el_cal_ch1_min_value, &el_cal_ch1_space, &el_cal_ch1_max_value}; 
M2_HLIST(el_cal_ch1, NULL, el_cal_ch1_list); 

M2_U32NUM(el_cal_ch2_min_value, "a0r1c4", &model.ch[1].min); 
M2_SPACE(el_cal_ch2_space, "W1h1");
M2_U32NUM(el_cal_ch2_max_value, "a0r1c4", &model.ch[1].max); 
M2_LIST(el_cal_ch2_list) = {&el_cal_ch2_min_value, &el_cal_ch2_space, &el_cal_ch2_max_value}; 
M2_HLIST(el_cal_ch2, NULL, el_cal_ch2_list); 

M2_U32NUM(el_cal_ch3_min_value, "a0r1c4", &model.ch[2].min); 
M2_SPACE(el_cal_ch3_space, "W1h1");
M2_U32NUM(el_cal_ch3_max_value, "a0r1c4", &model.ch[2].max); 
M2_LIST(el_cal_ch3_list) = {&el_cal_ch3_min_value, &el_cal_ch3_space, &el_cal_ch3_max_value}; 
M2_HLIST(el_cal_ch3, NULL, el_cal_ch3_list); 

M2_U32NUM(el_cal_ch4_min_value, "a0r1c4", &model.ch[3].min); 
M2_SPACE(el_cal_ch4_space, "W1h1");
M2_U32NUM(el_cal_ch4_max_value, "a0r1c4", &model.ch[3].max); 
M2_LIST(el_cal_ch4_list) = {&el_cal_ch4_min_value, &el_cal_ch4_space, &el_cal_ch4_max_value}; 
M2_HLIST(el_cal_ch4, NULL, el_cal_ch4_list); 

M2_U32NUM(el_cal_ch5_min_value, "a0r1c4", &model.ch[4].min); 
M2_SPACE(el_cal_ch5_space, "W1h1");
M2_U32NUM(el_cal_ch5_max_value, "a0r1c4", &model.ch[4].max); 
M2_LIST(el_cal_ch5_list) = {&el_cal_ch5_min_value, &el_cal_ch5_space, &el_cal_ch5_max_value}; 
M2_HLIST(el_cal_ch5, NULL, el_cal_ch5_list); 

M2_U32NUM(el_cal_ch6_min_value, "a0r1c4", &model.ch[5].min); 
M2_SPACE(el_cal_ch6_space, "W1h1");
M2_U32NUM(el_cal_ch6_max_value, "a0r1c4", &model.ch[5].max); 
M2_LIST(el_cal_ch6_list) = {&el_cal_ch6_min_value, &el_cal_ch6_space, &el_cal_ch6_max_value}; 
M2_HLIST(el_cal_ch6, NULL, el_cal_ch6_list); 

M2_LIST(el_cal_ci_list) = {
		&el_cal_ch1, 
		&el_cal_ch2, 
		&el_cal_ch3, 
		&el_cal_ch4, 
		&el_cal_ch5, 
		&el_cal_ch6
}; 
M2_VLIST(el_cal_ci, "x64y11l5w15", el_cal_ci_list); 

M2_EXTERN_ALIGN(top_el_sm); 

void fn_cal_btn_back(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_sm);
}
void fn_cal_btn_save(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
	model.calibrated = 1; 
  //m2_SetRoot(&top_el_im);
}

M2_BUTTON(el_cal_btn_back, "w30f4", "back", fn_cal_btn_back);
M2_BUTTON(el_cal_btn_save, "w30f4", "save", fn_cal_btn_save);
M2_SPACE(el_cal_btn_space, "W1h1");
M2_LIST(list_cal_btns) = {&el_cal_btn_back, &el_cal_btn_space, &el_cal_btn_save}; 
M2_HLIST(el_cal_btns, "x0y0", list_cal_btns); 

M2_LIST(list_cal_dialog) = {&el_cal_title, &el_cal_chi, &el_cal_ci, &el_cal_btns}; 
M2_XYLIST(el_cal_dialog, "l8", list_cal_dialog); 

M2_ALIGN(top_el_cal, "W64H64", &el_cal_dialog);
*/

//M2_ALIGN(top_el_im, "-1|2W64H64", &el_im_title_label); 

// MIX SETTINGS DIALOG


const char *fn_channel_picker_combo(uint8_t idx){
	switch(idx){
		case 0: return "OUT1"; 
		case 1: return "OUT2"; 
		case 2: return "OUT3"; 
		case 3: return "OUT4"; 
		case 4: return "OUT5"; 
		case 5: return "OUT6"; 
	}
	return " "; 
}

const char *fn_on_off_combo(uint8_t idx){
	switch(idx){
		case 0: return "OFF"; 
		case 1: return "ON"; 
	}
	return " "; 
}

M2_EXTERN_ALIGN(top_el_mix_settings); 

#include "gui_channel_setup_dlg.c"
#include "gui_profile_config_dlg.c"
#include "gui_setup_dlg.c"
#include "gui_mix_settings_dlg.c"
#include "gui_about_dlg.c"
#include "gui_main_dlg.c"
