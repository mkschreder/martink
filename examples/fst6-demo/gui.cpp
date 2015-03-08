
#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <gui/m2_fb.h>
#include "gui.hpp"

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
	uint8_t font;

	font = m2_el_fmfmt_get_font(fn_arg);
	switch(fn_arg->msg)
	{
		case M2_EL_MSG_IS_READ_ONLY:
			return 1;
		case M2_EL_MSG_GET_HEIGHT:
			return m2_gfx_add_readonly_border_height(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
		case M2_EL_MSG_GET_WIDTH:
			return m2_gfx_add_readonly_border_width(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_str_get_str(fn_arg))));
		case M2_EL_MSG_SHOW: {
			if ( m2_is_frame_draw_at_end == 0 )
				m2_el_fnfmt_fn(fn_arg);
				
			m2_pos_p b = (m2_pos_p)(fn_arg->data);
			
			uint16_t h = m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'h' ); 
			uint16_t w = m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'w' ); 
			
			if(w == 0) w = 4; if(h == 0) h = 20; 
			
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
	uint8_t font;

	font = m2_el_fmfmt_get_font(fn_arg);
	switch(fn_arg->msg)
	{
		case M2_EL_MSG_IS_READ_ONLY:
			return 1;
		case M2_EL_MSG_GET_HEIGHT:
			return m2_gfx_add_readonly_border_height(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_height(fn_arg, m2_gfx_get_char_height(font)));
		case M2_EL_MSG_GET_WIDTH:
			return m2_gfx_add_readonly_border_width(m2_el_fmfmt_opt_get_val_zero_default( fn_arg, 'b' ), font, m2_align_get_max_width(fn_arg, m2_gfx_get_text_width(font,m2_el_str_get_str(fn_arg))));
		case M2_EL_MSG_SHOW: {
			//m2_pos_p b = (m2_pos_p)(fn_arg->data);
			
			if ( m2_is_frame_draw_at_end == 0 )
				m2_el_fnfmt_fn(fn_arg);
			
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
/*
#define GUI_DIALOG_FRAME(name, title_content, fn_back, info_str, fn_save) \
	M2_BUTTON(_##name##_btn_settings, "-1w30f4", " <<< ", fn_back);\
	M2_BUTTON(_##name##_btn_info, "-1w30f4", " save ", fn_save);\
	M2_LABEL(_##name##_btn_space_label, "", info_str);\
	M2_ALIGN(_##name##_btn_space, "-1w55h6W64", &_##name##_btn_space_label); \
	M2_LIST(_##name##_list_sm_btns) = {&_##name##_btn_settings, &_##name##_btn_space, &_##name##_btn_info}; \
	M2_HLIST(_##name##_btns, "x0y0", _##name##_list_sm_btns); \
	M2_ALIGN(_##name##_title_align, "-1|2w126h62x1y1", title_content); \
	GUI_FRAME(_##name##_frame, "x0y0"); \
	M2_LIST(_##name##_list_dlg) = {&_##name##_title_align, &_##name##_frame, &_##name##_btns}; \
	M2_XYLIST(_##name##_list, "x0y0", _##name##_list_dlg);\
	M2_ALIGN(name, "x0y0-1|2W64H64", &_##name##_list); 
	//M2_ALIGN(_##name##_btns, "-1W64x0y0", &_##name##_btns_hlist); 
*/
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
/****************
* SETTINGS MENU
*****************/

M2_EXTERN_ALIGN(top_el_tlsm); 

/*
const char *el_sm_strlist_cb(uint8_t idx, uint8_t msg) {
	uint8_t select = msg == M2_STRLIST_MSG_SELECT; 
	switch(idx){
		case 0: 
			if(select) {
				model.calibrated = 0; 
				for(int c = 0; c < 6; c++){
					model.ch[c].min = 1500; 
					model.ch[c].max = 1500; 
				}
				//m2_SetRoot(&top_el_cal); 
			}
			return "Calibrate"; 
		//case 1: 
		//	if(select) m2_SetRoot(
		//	return 
	}
	return "---"; 
}
*/
uint8_t el_sm_first = 0;
uint8_t el_sm_cnt = 1;

M2_LABEL(el_sm_title_label, "", "===== settings ======");
M2_ALIGN(el_sm_title, "x0y53-1W64", &el_sm_title_label);

// options
	M2_LABEL(el_sm_name_label, "", "Name"); 
	M2_TEXT(el_sm_name, "a0", model.profile.name, sizeof(model.profile.name)); 

// MODE SELECT
	M2_LABEL(el_sm_mode_label, "", "Mode"); 

	const char *fn_sm_mode_select(uint8_t idx){
		switch(idx){
			case 0: return "mode 1"; 
			case 1: return "mode 2"; 
			case 2: return "mode 3"; 
			case 3: return "mode 4"; 
		}
		return " "; 
	}

	M2_COMBO(el_sm_mode, NULL, &model.profile.mode_id, 4, fn_sm_mode_select);
//===============
// CHANNELS 
		
		// LABELS
			
			// REVERSE CHANNEL
			M2_LABEL(el_chsm_reverse_label, "", "Reverse"); 

			const char *fn_chsm_reverse_select(uint8_t idx){
				switch(idx){
					case 0: return "no"; 
					case 1: return "yes"; 
				}
				return " "; 
			}

			M2_COMBO(el_chsm_reverse, NULL, &model.channel.reverse, 2, fn_chsm_reverse_select);
			// =================
			
			const char *fn_chsm_source(uint8_t idx){
				switch(idx){
					case 0: return "R stick H"; 
					case 1: return "R stick V"; 
					case 2: return "L stick V"; 
					case 3: return "L stick H"; 
					case 4: return "VRA"; 
					case 5: return "VRB"; 
					case 6: return "SWA"; 
					case 7: return "SWB"; 
					case 8: return "SWC"; 
					case 9: return "SWD"; 
				}
				return " "; 
			}

			M2_LABEL(el_chsm_source_label, "", "Source"); 
			M2_COMBO(el_chsm_source, NULL, &model.channel.source, 10, fn_chsm_source);

			M2_LABEL(el_chsm_raw_label, "", "Raw"); 
			M2_U32NUM(el_chsm_raw, "c4", &model.channel.input); 
			M2_LABEL(el_chsm_rate_label, "", "Rate"); 
			M2_U8NUM(el_chsm_rate, "c3", 0, 100, &model.channel.rate);
			M2_LABEL(el_chsm_exp_label, "", "Exponent"); 
			M2_U8NUM(el_chsm_exp, "c3", 0, 100, &model.channel.exponent);
			M2_LABEL(el_chsm_min_label, "", "Min"); 
			M2_U32NUM(el_chsm_min, "c4", &model.channel.min);
			M2_LABEL(el_chsm_max_label, "", "Max"); 
			M2_U32NUM(el_chsm_max, "c4", &model.channel.max);
			M2_LABEL(el_chsm_offset_label, "", "Offset"); 
			M2_U32NUM(el_chsm_offset, "c4", &model.channel.offset);
			M2_LABEL(el_chsm_out_label, "", "OUT"); 
			M2_U32NUM(el_chsm_out, "c4", &model.channel.output); 
			// =================
			// BOTTOM LINE
			M2_EXTERN_ALIGN(top_el_sm); 
			void fn_chsm_btn_back(m2_el_fnarg_p fnarg) {
				(void)fnarg; 
				//model.channel.request_save = 1; 
				m2_SetRoot(&top_el_tlsm);
			}
			void fn_chsm_btn_save(m2_el_fnarg_p fnarg) {
				(void)fnarg; 
				//fn_chsm_save_channel();
				//m2_SetRoot(&top_el_sm);
			}
			
			M2_BUTTON(el_chsm_btns, "x1y1f4-1w44", " <<< ", fn_chsm_btn_back);
			//M2_LIST(list_chsm_btns) = {&el_chsm_btn_back, &el_chsm_btn_space, &el_chsm_btn_save}; 
			//M2_HLIST(el_chsm_btns, "x0y0", list_chsm_btns); 
			
			// =================
			M2_LIST(list_chsm_gridlist) = {
				&el_chsm_source_label, &el_chsm_source,
				&el_chsm_raw_label, &el_chsm_raw, 
				&el_chsm_reverse_label, &el_chsm_reverse, 
				&el_chsm_rate_label, &el_chsm_rate,  
				&el_chsm_exp_label, &el_chsm_exp, 
				&el_chsm_min_label, &el_chsm_min, 
				&el_chsm_max_label, &el_chsm_max, 
				&el_chsm_offset_label, &el_chsm_offset, 
				&el_chsm_out_label, &el_chsm_out
			}; 
			M2_GRIDLIST(el_chsm_gridlist, "x10y11c2", list_chsm_gridlist); 
		
		GUI_CHANNEL(el_chsm_out_indicator, "x2y11w4h50", &chmin, &chmax, &chcenter, &model.channel.output); 

		// ===============
		M2_LIST(list_chsm) = {
			//&el_mm_channel_values, 
			//&el_mm_frame, 
			//&el_chsm_top_btns, 
			&el_chsm_out_indicator, 
			&el_chsm_gridlist, 
			&el_chsm_btns
		}; 
		M2_XYLIST(el_chsm_list, "", list_chsm); 
		M2_ALIGN(top_el_chsm, "x0y0-0|0W64H64", &el_chsm_list); 
		// ===============
		
	M2_LABEL(el_sm_chsm_label, "", "Channels"); 
	
	void fn_sm_chsm_btn(m2_el_fnarg_p fnarg) {
		(void)fnarg; 
		//fn_chsm_load_channel(0); 
		m2_SetRoot(&top_el_chsm);
	}
	
	M2_BUTTON(el_sm_chsm_btn, "", " >>> ", fn_sm_chsm_btn); 
	// ===============
	// MIXING
	M2_LABEL(el_sm_mixsm_label, "", "Mixing"); 
	
	void fn_sm_mixsm_btn(m2_el_fnarg_p fnarg) {
		(void)fnarg; 
		//m2_SetRoot(&top_el_tlsm);
	}
	
	M2_BUTTON(el_sm_mixsm_btn, "", " >>> ", fn_sm_mixsm_btn); 
	// ===============
	// CALIBRATION
	M2_LABEL(el_sm_cal_label, "", "Calibrate"); 
	
	void fn_sm_cal_btn(m2_el_fnarg_p fnarg) {
		(void)fnarg; 
		//m2_SetRoot(&top_el_cal);
	}
	
	M2_BUTTON(el_sm_cal_btn, "", " >>> ", fn_sm_cal_btn); 
	// ===============
	// BOTTOM LINE
	void fn_sm_btn_back(m2_el_fnarg_p fnarg) {
		(void)fnarg; 
		m2_SetRoot(&top_el_tlsm);
	}
	void fn_sm_btn_save(m2_el_fnarg_p fnarg) {
		(void)fnarg; 
		m2_SetRoot(&top_el_tlsm);
	}
	M2_BUTTON(el_sm_btn_settings, "-1w30f4", " <<< ", fn_sm_btn_back);
	M2_BUTTON(el_sm_btn_info, "-1w30f4", " save ", fn_sm_btn_save);
	M2_LABEL(el_sm_btn_space_label, "", model.profile.name);
	M2_ALIGN(el_sm_btn_space, "-1w55h6W64", &el_sm_btn_space_label); 
	M2_LIST(list_sm_btns) = {&el_sm_btn_settings, &el_sm_btn_space, &el_sm_btn_info}; 
	M2_HLIST(el_sm_btns_hlist, "", list_sm_btns); 
	M2_ALIGN(el_sm_btns, "-1|W64x0y0", &el_sm_btns_hlist); 

// ===============
// ASSEMBLED SETTINGS MENU
	M2_LIST(list_sm_gridlist_left) = {
		&el_sm_name_label, &el_sm_name, 
		&el_sm_mode_label, &el_sm_mode, 
		&el_sm_chsm_label, &el_sm_chsm_btn, 
		&el_sm_mixsm_label, &el_sm_mixsm_btn, 
		&el_sm_cal_label, &el_sm_cal_btn
	}; 
	M2_GRIDLIST(el_sm_gridlist_left, "x1y11c2", list_sm_gridlist_left); 
	
	//M2_STRLIST(el_sm_strlist, "y1l6W56t1", &el_sm_first, &el_sm_cnt, el_sm_strlist_cb);
	//M2_VSB(el_sm_vsb, "x120y14l6w4r1t1", &el_sm_first, &el_sm_cnt);
	//M2_LIST(list_sm_strlist) = { &el_sm_title, &el_sm_strlist, &el_sm_vsb, &el_sm_btns };
	M2_LIST(list_sm_strlist) = { &el_sm_title, &el_sm_gridlist_left, &el_sm_btns };
	M2_XYLIST(el_sm_list, NULL, list_sm_strlist);
	M2_ALIGN(top_el_sm, "-1|2W64H64", &el_sm_list); 
// ===============

/****************
* ABOUT MENU
*****************/

M2_LABEL(el_im_title_label, "-1", "about");
M2_ALIGN(top_el_im, "-1|2W64H64", &el_im_title_label); 

/****************
* MAIN SCREEN 
*****************/

void fn_mm_btn_settings(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_sm);
}
void fn_mm_btn_info(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_im);
}
void fn_mm_btn_m1(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	strcpy(model.profile.name, "m1"); 
}
void fn_mm_btn_m2(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	strcpy(model.profile.name, "m2"); 
}
void fn_mm_btn_m3(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	strcpy(model.profile.name, "m3"); 
}
void fn_mm_btn_m4(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	strcpy(model.profile.name, "m4"); 
}
void fn_mm_btn_m5(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	strcpy(model.profile.name, "m5"); 
}
void fn_mm_btn_m6(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	strcpy(model.profile.name, "m6"); 
}

//M2_LABEL(el_mm_title_label, "", "======= FS-T6 =======");
//M2_ALIGN(el_mm_title, "x0y56-1|2W64", &el_mm_title_label); 
/*
const char *fn_mm_mode_list(uint8_t idx, uint8_t msg) {
	uint8_t select = msg == M2_STRLIST_MSG_SELECT; 
	static const char *titles[] = {"m1", "m2", "m3", "m4", "m5", "m6"}; 
	if(idx >= 6) return " "; 
	if(select){
		strcpy(model.profile.name, titles[idx]); 
	}
	return titles[idx]; 
}
static uint8_t el_mm_mode_list_count = 6; 

M2_STRLIST(el_mm_mode_list, "a0", &model.mode, &el_mm_mode_list_count, fn_mm_mode_list); 
*/

// the top model buttons
M2_BUTTON(el_mm_btn_m1, "", " m1 ", fn_mm_btn_m1); 
M2_BUTTON(el_mm_btn_m2, "", " m2 ", fn_mm_btn_m2); 
M2_BUTTON(el_mm_btn_m3, "", " m3 ", fn_mm_btn_m3); 
M2_BUTTON(el_mm_btn_m4, "", " m4 ", fn_mm_btn_m4); 
M2_BUTTON(el_mm_btn_m5, "", " m5 ", fn_mm_btn_m5); 
M2_BUTTON(el_mm_btn_m6, "", " m6 ", fn_mm_btn_m6); 
M2_LIST(list_mm_top_btns) = {&el_mm_btn_m1, &el_mm_btn_m2, &el_mm_btn_m3, &el_mm_btn_m4, &el_mm_btn_m5, &el_mm_btn_m6}; 
M2_HLIST(el_mm_top_btns_hlist, "a0", list_mm_top_btns); 
M2_ALIGN(el_mm_top_btns, "-1|2w126h62x1y1", &el_mm_top_btns_hlist); 

M2_BUTTON(el_mm_btn_settings, "-1w30f4", "  about ", fn_mm_btn_info);
M2_BUTTON(el_mm_btn_info, "-1w30f4", "  setup ", fn_mm_btn_settings);
M2_LABEL(el_mm_btn_space_label, "", model.profile.name);
M2_ALIGN(el_mm_btn_space, "-1|0w55h6W64", &el_mm_btn_space_label); 
M2_LIST(list_mm_btns) = {&el_mm_btn_settings, &el_mm_btn_space, &el_mm_btn_info}; 
M2_HLIST(el_mm_btns_hlist, "", list_mm_btns); 
M2_ALIGN(el_mm_btns, "-1|W64x0y0", &el_mm_btns_hlist); 

static void fn_chsm_go(int ch){
	model.channel.id = ch; 
	model.channel.request_load = 1; 
	m2_SetRoot(&top_el_chsm); 
}

// the channel info control
void fn_mm_btn_ch1(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(0); 
}

M2_BUTTON(el_mm_ch1_label, "", "OUT1", fn_mm_btn_ch1); 
M2_U32NUM(el_mm_ch1_value, "a0r1c4", &model.out[0].value); 

void fn_mm_btn_ch2(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(1); 
}

M2_BUTTON(el_mm_ch2_label, "", "OUT2", fn_mm_btn_ch2); 
M2_U32NUM(el_mm_ch2_value, "a0r1c4", &model.out[1].value); 


void fn_mm_btn_ch3(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(2); 
}

M2_BUTTON(el_mm_ch3_label, "", "OUT3", fn_mm_btn_ch3); 
M2_U32NUM(el_mm_ch3_value, "a0r1c4", &model.out[2].value); 

void fn_mm_btn_ch4(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(3); 
}

M2_BUTTON(el_mm_ch4_label, "", "OUT4", fn_mm_btn_ch4); 
M2_U32NUM(el_mm_ch4_value, "a0r1c4", &model.out[3].value);
 
void fn_mm_btn_ch5(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(4); 
}

M2_BUTTON(el_mm_ch5_label, "", "OUT5", fn_mm_btn_ch5); 
M2_U32NUM(el_mm_ch5_value, "a0r1c4", &model.out[4].value); 

void fn_mm_btn_ch6(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(5); 
}

M2_BUTTON(el_mm_ch6_label, "", "OUT6", fn_mm_btn_ch6); 
M2_U32NUM(el_mm_ch6_value, "a0r1c4", &model.out[5].value); 

/*
const char *fn_mm_chan_source(uint8_t idx){
	switch(idx){
		case 0: return "L stick H"; 
		case 1: return "L stick V"; 
		case 2: return "R stick V"; 
		case 3: return "R stick H"; 
		case 4: return "VRA"; 
		case 5: return "VRB"; 
		case 6: return "SWA"; 
		case 7: return "SWB"; 
		case 8: return "SWC"; 
		case 9: return "SWD"; 
	}
	return " "; 
}

M2_COMBO(el_mm_ch1_src, NULL, &model.ch[0].source, 10, fn_mm_chan_source);
M2_COMBO(el_mm_ch2_src, NULL, &model.ch[1].source, 10, fn_mm_chan_source);
M2_COMBO(el_mm_ch3_src, NULL, &model.ch[2].source, 10, fn_mm_chan_source);
M2_COMBO(el_mm_ch4_src, NULL, &model.ch[3].source, 10, fn_mm_chan_source);
M2_COMBO(el_mm_ch5_src, NULL, &model.ch[4].source, 10, fn_mm_chan_source);
M2_COMBO(el_mm_ch6_src, NULL, &model.ch[5].source, 10, fn_mm_chan_source);
*/
M2_LIST(list_mm_chi) = {
		&el_mm_ch1_label, &el_mm_ch1_value,
		&el_mm_ch2_label, &el_mm_ch2_value, 
		&el_mm_ch3_label, &el_mm_ch3_value, 
		&el_mm_ch4_label, &el_mm_ch4_value,
		&el_mm_ch5_label, &el_mm_ch5_value, 
		&el_mm_ch6_label, &el_mm_ch6_value,
}; 
M2_GRIDLIST(el_mm_chi_list, "a0x8y16c2W25", list_mm_chi); 
M2_ALIGN(el_mm_chi, "x8y16-0|0W64", &el_mm_chi_list); 

// MIX SETTINGS DIALOG
M2_LABEL(el_chsm_reverse_label, "", "Reverse"); 

const char *fn_chsm_reverse_select(uint8_t idx){
	switch(idx){
		case 0: return "no"; 
		case 1: return "yes"; 
	}
	return " "; 
}

M2_COMBO(el_chsm_reverse, NULL, &model.channel.reverse, 2, fn_chsm_reverse_select);
// =================

const char *fn_mixsm_combo(uint8_t idx){
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

M2_LABEL(el_mixsm_master_label, "", "Master"); 
M2_COMBO(el_mixsm_master, NULL, &model.mix.master_channel, 6, fn_mixsm_combo);
M2_LABEL(el_mixsm_slave_label, "", "Slave"); 
M2_COMBO(el_mixsm_slave, NULL, &model.mix.slave_channel, 6, fn_mixsm_combo);

M2_LABEL(el_mixsm_pos_label, "", "Pos. Mix"); 
M2_U8NUM(el_mixsm_pos, "c4", 0, 100, &model.mix.pos_mix); 
M2_LABEL(el_mixsm_neg_label, "", "Neg. Mix"); 
M2_U8NUM(el_mixsm_neg, "c4", 0, 100, &model.mix.neg_mix); 
M2_LABEL(el_mixsm_offset_label, "", "Offset"); 
M2_U8NUM(el_mixsm_offset, "c4", 0, 100, &model.mix.offset);

void fn_mixsm_btn_back(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
	//model.channel.request_save = 1; 
	m2_SetRoot(&top_el_tlsm);
}

M2_BUTTON(el_mixsm_back_btn, "x1y1f4-1w44", " <<< ", fn_mixsm_btn_back);
//M2_LIST(list_chsm_btns) = {&el_chsm_btn_back, &el_chsm_btn_space, &el_chsm_btn_save}; 
//M2_HLIST(el_chsm_btns, "x0y0", list_chsm_btns); 

// =================
M2_LIST(list_chsm_gridlist) = {
	&el_mixsm_master_label, &el_mixsm_master,
	&el_mixsm_slave_label, &el_mixsm_slave, 
	&el_mixsm_pos_label, &el_mixsm_pos, 
	&el_mixsm_neg_label, &el_mixsm_neg,  
	&el_mixsm_offset_label, &el_mixsm_offset
}; 
M2_GRIDLIST(el_mixsm_gridlist, "x10y11c2", list_mixsm_gridlist); 

// ===============
M2_LIST(list_mixsm) = {
	//&el_mm_channel_values, 
	//&el_mm_frame, 
	//&el_chsm_top_btns, 
	&el_mixsm_gridlist, 
	&el_mixsm_back_btn
}; 
M2_XYLIST(el_mixsm_list, "", list_chsm); 
M2_ALIGN(top_el_mix_settings, "x0y0-0|0W64H64", &el_chsm_list); 

// ==================

// MIX SELECITON PART
	M2_LABEL(el_mm_fst6_label_text, "f1", "KT-6");
	M2_ALIGN(el_mm_fst6_label, "-1|0W64h7", &el_mm_fst6_label_text); 
	M2_SPACE(el_mm_fst6_space, "w1h1");
	
	GUI_SWITCH(el_mm_swa_value, "w4h6b1", &model.sw[0]); 
	GUI_SWITCH(el_mm_swb_value, "w4h6b1", &model.sw[1]); 
	GUI_SWITCH(el_mm_swc_value, "w4h6b1", &model.sw[2]); 
	GUI_SWITCH(el_mm_swd_value, "w4h6b1", &model.sw[3]); 
	M2_LIST(list_mm_sw) = {
		&el_mm_swd_value, &el_mm_swb_value, &el_mm_swa_value, &el_mm_swc_value
	}; 
	M2_GRIDLIST(el_mm_sw_gridlist, "c4", list_mm_sw); 
	M2_ALIGN(el_mm_sw_list, "-1|0W64h9", &el_mm_sw_gridlist); 
	
	M2_BUTTON(el_mm_calibrate_btn, "f4w32", "calib", NULL); 
	M2_ALIGN(el_mm_calibrate, "-1|0W64h9", &el_mm_calibrate_btn); 
	M2_BUTTON(el_mm_reset_btn, "f4w32", "reset", NULL); 
	M2_ALIGN(el_mm_reset, "-1|0W64h9", &el_mm_reset_btn); 
	
	M2_LIST(list_mm_middle) = {
		&el_mm_fst6_label, 
		&el_mm_fst6_space,
		&el_mm_sw_list, 
		&el_mm_calibrate, 
		&el_mm_fst6_space, 
		&el_mm_reset
	}; 
	M2_VLIST(el_mm_sw_vlist, "", list_mm_middle); 
	M2_ALIGN(el_mm_center, "-1|2x0y12W64h42", &el_mm_sw_vlist); 
	
	#define MULTI_SELECT_CNT 3
	const char *multi_select_strings[MULTI_SELECT_CNT] = { "mix 1", "mix 2", "mix 3" };
	
	uint8_t el_muse_first = 0;
	uint8_t el_muse_cnt = MULTI_SELECT_CNT;

	const char *el_muse_strlist_cb(uint8_t idx, uint8_t msg) {
		const char *s = "";
		if ( msg == M2_STRLIST_MSG_SELECT ) {
			if ( model.mix_enabled[idx] == 0 ) {
				model.mix_enabled[idx] = 1;
			}
			else {
				model.mix_enabled[idx].on = 0;
			}
		}
		if ( msg == M2_STRLIST_MSG_GET_STR ) {
			s = multi_select_strings[idx];
		}
		if ( msg == M2_STRLIST_MSG_GET_EXTENDED_STR ) {
			if ( model.mix_enabled[idx] == 0 ) {
				s = " ";
			}
			else {
				s = "*";
			}
		}
		return s;  
	}
	
	void fn_mix_1_btn(m2_el_fnarg_p fnarg) { 
		(void)fnarg; 
		model.mix.id = 0; 
		model.mix.request_load = 1; 
		m2_SetRoot(&top_el_mix_settings); 
	}
	void fn_mix_2_btn(m2_el_fnarg_p fnarg) { 
		(void)fnarg;  
		model.mix.id = 0; 
		model.mix.request_load = 1; 
		m2_SetRoot(&top_el_mix_settings); 
	}
	void fn_mix_3_btn(m2_el_fnarg_p fnarg) { 
		(void)fnarg;  
		model.mix.id = 0; 
		model.mix.request_load = 1; 
		m2_SetRoot(&top_el_mix_settings); 
	}
	M2_STRLIST(el_muse_strlist, "l3E2W10", &el_muse_first, &el_muse_cnt, el_muse_strlist_cb);
	M2_BUTTON(el_mix_1_btn, "", " >>> ", fn_mix_1_btn); 
	M2_BUTTON(el_mix_2_btn, "", " >>> ", fn_mix_2_btn); 
	M2_BUTTON(el_mix_3_btn, "", " >>> ", fn_mix_3_btn); 
	M2_LIST(list_mm_mix_btns) = {
		&el_mix_1_btn, &el_mix_2_btn, &el_mix_3_btn
	}; 
	M2_VLIST(el_mm_mix_btns, "", list_mm_mix_btns); 
	
	M2_LIST(list_mm_mix) = {
		&el_muse_strlist, 
		&el_mm_mix_btns
	}; 
	M2_HLIST(el_mm_mix_list, "a0", list_mm_mix); 
	
	const char *fn_mm_armed(uint8_t idx){
		switch(idx){
			case 0: return "ARMED"; 
			case 1: return "OFF"; 
		}
		return " "; 
	}

	M2_COMBO(el_mm_armed_combo, NULL, &model.armed, 2, fn_mm_armed);
	M2_ALIGN(el_mm_armed, "-1|1W64h9", &el_mm_armed_combo); 
	
	// right hand side controls
	M2_LIST(list_mm_right_panel) = {
		&el_mm_armed,
		&el_mm_mix_list
	}; 
	M2_VLIST(el_mm_mixes_list, "", list_mm_right_panel); 
	M2_ALIGN(el_mm_mixes, "-0|0x81y16W64H64", &el_mm_mixes_list); 
	
	//M2_ALIGN(el_mm_mixes, "-0|1x81y0W64H64", &el_mm_right_panel); 
	/*
	M2_LABEL(el_mm_swi_list_label, "-1", "  aux "); 
	M2_LIST(el_mm_swi_list) = {
			&el_mm_swi_list_label, 
			&el_mm_sw,
			&el_mm_mix_list
	}; 
	
	M2_VLIST(el_mm_mixes, "x60y16w15", el_mm_swi_list); */
// ================

//M2_BOX(el_main_box, "w128h64x0y0"); 


// horizontal left
GUI_CHANNEL(el_mm_ch3_sl, "x2y14w2h40", &chmin, &chmax, &chcenter, &model.inputs[2].value); 
GUI_CHANNEL(el_mm_ch4_sl, "x6y12w40h2", &chmin, &chmax, &chcenter, &model.inputs[3].value); 

GUI_CHANNEL(el_mm_ch2_sl, "x123y14w2h40", &chmin, &chmax, &chcenter, &model.inputs[1].value); 
GUI_CHANNEL(el_mm_ch1_sl, "x81y12w40h2", &chmin, &chmax, &chcenter, &model.inputs[0].value); 

GUI_CHANNEL(el_mm_ch5_sl, "x77y14w2h40", &chmin, &chmax, &chcenter, &model.inputs[4].value); 
GUI_CHANNEL(el_mm_ch6_sl, "x48y14w2h40", &chmin, &chmax, &chcenter, &model.inputs[5].value); 

M2_LIST(list_mm_channel_values) = {
	&el_mm_ch1_sl, &el_mm_ch2_sl, &el_mm_ch3_sl, &el_mm_ch4_sl, &el_mm_ch5_sl, &el_mm_ch6_sl
}; 
M2_XYLIST(el_mm_channel_values, "W64H64", list_mm_channel_values); 

GUI_FRAME(el_mm_frame, "x0y0"); 

// place the title and the buttons and other controls into a vertical list
M2_LIST(list_mm_dialog) = {
		&el_mm_top_btns, 
		&el_mm_chi, 
		&el_mm_center, 
		&el_mm_mixes,
		&el_mm_btns,
		&el_mm_frame,
		&el_mm_channel_values, 
		//&el_mm_swi
}; 
M2_XYLIST(el_mm_dialog, "", list_mm_dialog); 

M2_ALIGN(top_el_tlsm, "W64H64", &el_mm_dialog);

void gui_init(fbuf_dev_t fb){
	
	m2_fb_init(fb, &top_el_tlsm);
	
	//m2_tty_init(screen, &top_el_tlsm); 

}

struct gui_data* gui_get_data(void){
	return &model; 
}
