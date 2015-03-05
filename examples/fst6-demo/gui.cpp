
#include <kernel.h>
#include <boards/rc/flysky-t6-tx.h>
#include <gui/m2_fb.h>
#include "gui.hpp"

static struct gui_model model; 

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
	return 0; //m2_el_str_fn(fn_arg);
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
			
			m2_gfx_hline(0, 63, 128); // top
			m2_gfx_hline(0, 54, 128); // top +1 
			m2_gfx_hline(0, 9, 128); // bottom -1
			m2_gfx_hline(0, 0, 128); // bottom
			
			m2_gfx_vline(0, 63, 64); // left
			m2_gfx_vline(127, 63, 64); // right
			
			if ( m2_is_frame_draw_at_end != 0 )
				m2_el_fnfmt_fn(fn_arg);
		} return 1;    
	}
	return 0; //m2_el_str_fn(fn_arg);
}
#define GUI_FRAME(el,fmt) M2_EL_CONST m2_el_frame_t el M2_SECTION_PROGMEM = { { m2_el_frame_fn, (fmt) } }
#define GUI_EXTERN_FRAME(el) extern M2_EL_CONST m2_el_frame_t el

/****************
* CALIBRATION DIALOG
*****************/

M2_LABEL(el_cal_title_label, "-1", "===== calibrate =====");
M2_ALIGN(el_cal_title, "x0y8-1|2W64H64", &el_cal_title_label); 

M2_EXTERN_HLIST(el_mm_ch1); 
M2_EXTERN_HLIST(el_mm_ch2); 
M2_EXTERN_HLIST(el_mm_ch3); 
M2_EXTERN_HLIST(el_mm_ch4); 
M2_EXTERN_HLIST(el_mm_ch5); 
M2_EXTERN_HLIST(el_mm_ch6); 

M2_LIST(el_cal_chi_list) = {
		&el_mm_ch1, 
		&el_mm_ch2, 
		&el_mm_ch3, 
		&el_mm_ch4, 
		&el_mm_ch5, 
		&el_mm_ch6
}; 
M2_VLIST(el_cal_chi, "x0y1l5w15", el_cal_chi_list); 

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
M2_VLIST(el_cal_ci, "x10y1l5w15", el_cal_ci_list); 

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

M2_BUTTON(el_cal_btn_back, "w10f4", "back", fn_cal_btn_back);
M2_BUTTON(el_cal_btn_save, "w10f4", "save", fn_cal_btn_save);
M2_SPACE(el_cal_btn_space, "W1h1");
M2_LIST(list_cal_btns) = {&el_cal_btn_back, &el_cal_btn_space, &el_cal_btn_save}; 
M2_HLIST(el_cal_btns, "x0y0", list_cal_btns); 

M2_LIST(list_cal_dialog) = {&el_cal_title, &el_cal_chi, &el_cal_ci, &el_cal_btns}; 
M2_XYLIST(el_cal_dialog, "l8", list_cal_dialog); 

M2_ALIGN(top_el_cal, "W64H64", &el_cal_dialog);

/****************
* SETTINGS MENU
*****************/

M2_EXTERN_ALIGN(top_el_tlsm); 

void fn_sm_btn_back(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_tlsm);
}

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
				m2_SetRoot(&top_el_cal); 
			}
			return "Calibrate"; 
		//case 1: 
		//	if(select) m2_SetRoot(
		//	return 
	}
	return "---"; 
}

uint8_t el_sm_first = 0;
uint8_t el_sm_cnt = 1;

M2_LABEL(el_sm_title_label, "-1", "===== settings ======");
M2_ALIGN(el_sm_title, "x0y56-1|2W64H64", &el_sm_title_label); 

M2_BUTTON(el_sm_btn_back, "-1W32f4", "back", fn_sm_btn_back);
M2_SPACE(el_sm_btn_space, "W1h1");
M2_LIST(list_sm_btns) = {&el_sm_btn_back, &el_sm_btn_space}; 
M2_HLIST(el_sm_btns, "W64x0y0", list_sm_btns); 

M2_STRLIST(el_sm_strlist, "y1l6W56t1", &el_sm_first, &el_sm_cnt, el_sm_strlist_cb);
M2_VSB(el_sm_vsb, "x120y14l6w4r1t1", &el_sm_first, &el_sm_cnt);
M2_LIST(list_sm_strlist) = { &el_sm_title, &el_sm_strlist, &el_sm_vsb, &el_sm_btns };
M2_XYLIST(el_sm_list, NULL, list_sm_strlist);
M2_ALIGN(top_el_sm, "-1|2W64H64", &el_sm_list); 

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
M2_LABEL(el_mm_title_label, "-1", "======= FS-T6 =======");
M2_ALIGN(el_mm_title, "x0y56-1|2W64H64", &el_mm_title_label); 

// the top model buttons
M2_BUTTON(el_mm_btn_m1, "", " m1 ", fn_mm_btn_info); 
M2_BUTTON(el_mm_btn_m2, "", " m2 ", fn_mm_btn_info); 
M2_BUTTON(el_mm_btn_m3, "", " m3 ", fn_mm_btn_info); 
M2_BUTTON(el_mm_btn_m4, "", " m4 ", fn_mm_btn_info); 
M2_BUTTON(el_mm_btn_m5, "", " m5 ", fn_mm_btn_info); 
M2_BUTTON(el_mm_btn_m6, "", " m6 ", fn_mm_btn_info); 
M2_LIST(list_mm_top_btns) = {&el_mm_btn_m1, &el_mm_btn_m2, &el_mm_btn_m3, &el_mm_btn_m4, &el_mm_btn_m5, &el_mm_btn_m6}; 
M2_HLIST(el_mm_top_btns_hlist, "", list_mm_top_btns); 
M2_ALIGN(el_mm_top_btns, "-1|2w126h61x1y1", &el_mm_top_btns_hlist); 

M2_BUTTON(el_mm_btn_settings, "-1w44", "  about ", fn_mm_btn_info);
M2_BUTTON(el_mm_btn_info, "-1w44", "  conf ", fn_mm_btn_settings);
M2_SPACE(el_mm_btn_space, "w38h1");
M2_LIST(list_mm_btns) = {&el_mm_btn_settings, &el_mm_btn_space, &el_mm_btn_info}; 
M2_HLIST(el_mm_btns, "x0y0", list_mm_btns); 

// the channel info control
M2_LABEL(el_mm_ch1_label, NULL, "CH1"); 
M2_U32NUM(el_mm_ch1_value, "a0r1c4", &model.ch[0].value); 
M2_LIST(el_mm_ch1_list) = {&el_mm_ch1_label, &el_mm_ch1_value}; 
M2_HLIST(el_mm_ch1, NULL, el_mm_ch1_list); 

M2_LABEL(el_mm_ch2_label, NULL, "CH2"); 
M2_U32NUM(el_mm_ch2_value, "a0r1c4", &model.ch[1].value); 
M2_LIST(el_mm_ch2_list) = {&el_mm_ch2_label, &el_mm_ch2_value}; 
M2_HLIST(el_mm_ch2, NULL, el_mm_ch2_list); 

M2_LABEL(el_mm_ch3_label, NULL, "CH3"); 
M2_U32NUM(el_mm_ch3_value, "a0r1c4", &model.ch[2].value); 
M2_LIST(el_mm_ch3_list) = {&el_mm_ch3_label, &el_mm_ch3_value}; 
M2_HLIST(el_mm_ch3, NULL, el_mm_ch3_list); 

M2_LABEL(el_mm_ch4_label, NULL, "CH4"); 
M2_U32NUM(el_mm_ch4_value, "a0r1c4", &model.ch[3].value);
M2_LIST(el_mm_ch4_list) = {&el_mm_ch4_label, &el_mm_ch4_value}; 
M2_HLIST(el_mm_ch4, NULL, el_mm_ch4_list); 
 
M2_LABEL(el_mm_ch5_label, NULL, "CH5"); 
M2_U32NUM(el_mm_ch5_value, "a0r1c4", &model.ch[4].value); 
M2_LIST(el_mm_ch5_list) = {&el_mm_ch5_label, &el_mm_ch5_value}; 
M2_HLIST(el_mm_ch5, NULL, el_mm_ch5_list); 

M2_LABEL(el_mm_ch6_label, NULL, "CH6"); 
M2_U32NUM(el_mm_ch6_value, "a0r1c4", &model.ch[5].value); 
M2_LIST(el_mm_ch6_list) = {&el_mm_ch6_label, &el_mm_ch6_value}; 
M2_HLIST(el_mm_ch6, NULL, el_mm_ch6_list); 

static char el_mm_swab_str[8]; 
static char el_mm_swcd_str[8]; 

M2_LABEL(el_mm_swab_strlist, NULL, el_mm_swab_str); 
M2_LABEL(el_mm_swcd_strlist, NULL, el_mm_swcd_str); 

M2_LABEL(el_mm_swab_label, NULL, "A/B"); 
M2_LIST(el_mm_swab_list) = {&el_mm_swab_label, &el_mm_swab_strlist}; 
M2_HLIST(el_mm_swab, NULL, el_mm_swab_list); 

M2_LABEL(el_mm_swcd_label, NULL, "C/D");  
M2_LIST(el_mm_swcd_list) = {&el_mm_swcd_label, &el_mm_swcd_strlist}; 
M2_HLIST(el_mm_swcd, NULL, el_mm_swcd_list);

M2_LABEL(el_mm_chi_list_label, "-1", " sticks "); 
M2_LIST(el_mm_chi_list) = {
		&el_mm_chi_list_label, 
		&el_mm_ch1, 
		&el_mm_ch2, 
		&el_mm_ch3, 
		&el_mm_ch4
}; 
M2_VLIST(el_mm_chi, "x8y14l5w15", el_mm_chi_list); 

M2_LABEL(el_mm_swi_list_label, "-1", " aux "); 
M2_LIST(el_mm_swi_list) = {
		&el_mm_swi_list_label, 
		&el_mm_ch5, 
		&el_mm_ch6, 
		&el_mm_swab, 
		&el_mm_swcd
}; 
M2_VLIST(el_mm_swi, "x64y14l5w15", el_mm_swi_list); 

//M2_BOX(el_main_box, "w128h64x0y0"); 

static uint32_t chmin = 1000, chmax = 2000, chcenter = 1500; 

// horizontal left
GUI_CHANNEL(el_mm_ch3_sl, "x2y11w2h40", &chmin, &chmax, &chcenter, &model.ch[2].value); 
GUI_CHANNEL(el_mm_ch4_sl, "x6y11w40h2", &chmin, &chmax, &chcenter, &model.ch[3].value); 

GUI_CHANNEL(el_mm_ch2_sl, "x123y11w2h40", &chmin, &chmax, &chcenter, &model.ch[1].value); 
GUI_CHANNEL(el_mm_ch1_sl, "x81y11w40h2", &chmin, &chmax, &chcenter, &model.ch[0].value); 

GUI_FRAME(el_mm_frame, "x0y0"); 

// place the title and the buttons and other controls into a vertical list
M2_LIST(list_mm_dialog) = {&el_mm_title, &el_mm_frame, &el_mm_top_btns, 
		&el_mm_ch1_sl,&el_mm_ch2_sl,&el_mm_ch3_sl,&el_mm_ch4_sl,
		&el_mm_chi, &el_mm_swi, &el_mm_btns}; 
M2_XYLIST(el_mm_dialog, "", list_mm_dialog); 

M2_ALIGN(top_el_tlsm, "W64H64", &el_mm_dialog);

void gui_init(fbuf_dev_t fb){
	
	m2_fb_init(fb, &top_el_tlsm);
	
	//m2_tty_init(screen, &top_el_tlsm); 
	
	for(int c = 0; c < 6; c++){
		model.ch[c].min = 1500; 
		model.ch[c].max = 1500; 
	}
}

struct gui_model* gui_get_model(void){
	return &model; 
}
