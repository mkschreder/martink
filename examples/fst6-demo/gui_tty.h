 

struct channel_info {
	uint32_t ch[6]; 
	uint32_t range[6][2]; 
	uint8_t calibrated; 
	uint8_t swa, swb, swc, swd; 
} channel_info; 

/****************
* CALIBRATION DIALOG
*****************/

M2_LABEL(el_cal_title_label, "-1", "\x1b[30;47m===== calibrate =====\x1b[m");
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

M2_U32NUM(el_cal_ch1_min_value, "a0r1c4", &channel_info.range[0][0]); 
M2_SPACE(el_cal_ch1_space, "W1h1");
M2_U32NUM(el_cal_ch1_max_value, "a0r1c4", &channel_info.range[0][1]); 
M2_LIST(el_cal_ch1_list) = {&el_cal_ch1_min_value, &el_cal_ch1_space, &el_cal_ch1_max_value}; 
M2_HLIST(el_cal_ch1, NULL, el_cal_ch1_list); 

M2_U32NUM(el_cal_ch2_min_value, "a0r1c4", &channel_info.range[1][0]); 
M2_SPACE(el_cal_ch2_space, "W1h1");
M2_U32NUM(el_cal_ch2_max_value, "a0r1c4", &channel_info.range[1][1]); 
M2_LIST(el_cal_ch2_list) = {&el_cal_ch2_min_value, &el_cal_ch2_space, &el_cal_ch2_max_value}; 
M2_HLIST(el_cal_ch2, NULL, el_cal_ch2_list); 

M2_U32NUM(el_cal_ch3_min_value, "a0r1c4", &channel_info.range[2][0]); 
M2_SPACE(el_cal_ch3_space, "W1h1");
M2_U32NUM(el_cal_ch3_max_value, "a0r1c4", &channel_info.range[2][1]); 
M2_LIST(el_cal_ch3_list) = {&el_cal_ch3_min_value, &el_cal_ch3_space, &el_cal_ch3_max_value}; 
M2_HLIST(el_cal_ch3, NULL, el_cal_ch3_list); 

M2_U32NUM(el_cal_ch4_min_value, "a0r1c4", &channel_info.range[3][0]); 
M2_SPACE(el_cal_ch4_space, "W1h1");
M2_U32NUM(el_cal_ch4_max_value, "a0r1c4", &channel_info.range[3][1]); 
M2_LIST(el_cal_ch4_list) = {&el_cal_ch4_min_value, &el_cal_ch4_space, &el_cal_ch4_max_value}; 
M2_HLIST(el_cal_ch4, NULL, el_cal_ch4_list); 

M2_U32NUM(el_cal_ch5_min_value, "a0r1c4", &channel_info.range[4][0]); 
M2_SPACE(el_cal_ch5_space, "W1h1");
M2_U32NUM(el_cal_ch5_max_value, "a0r1c4", &channel_info.range[4][1]); 
M2_LIST(el_cal_ch5_list) = {&el_cal_ch5_min_value, &el_cal_ch5_space, &el_cal_ch5_max_value}; 
M2_HLIST(el_cal_ch5, NULL, el_cal_ch5_list); 

M2_U32NUM(el_cal_ch6_min_value, "a0r1c4", &channel_info.range[5][0]); 
M2_SPACE(el_cal_ch6_space, "W1h1");
M2_U32NUM(el_cal_ch6_max_value, "a0r1c4", &channel_info.range[5][1]); 
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
  channel_info.calibrated = 1; 
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
				channel_info.calibrated = 0; 
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

M2_LABEL(el_sm_title_label, "-1", "\x1b[30;47m===== settings ======\x1b[m");
M2_ALIGN(el_sm_title, "x0y8-1|2W64H64", &el_sm_title_label); 

M2_BUTTON(el_sm_btn_back, "-1W32f4", "back", fn_sm_btn_back);
M2_SPACE(el_sm_btn_space, "W1h1");
M2_LIST(list_sm_btns) = {&el_sm_btn_back, &el_sm_btn_space}; 
M2_HLIST(el_sm_btns, "W64x0y0", list_sm_btns); 

M2_STRLIST(el_sm_strlist, "y1l6W56t1", &el_sm_first, &el_sm_cnt, el_sm_strlist_cb);
M2_VSB(el_sm_vsb, "x20y1l6W4r1t1", &el_sm_first, &el_sm_cnt);
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
M2_LABEL(el_mm_title_label, "-1", "\x1b[30;47m======= FS-T6 =======\x1b[m");
M2_ALIGN(el_mm_title, "x0y8-1|2W64H64", &el_mm_title_label); 

M2_BUTTON(el_mm_btn_settings, "w10f4", "settings", fn_mm_btn_settings);
M2_BUTTON(el_mm_btn_info, "w10f4", "about", fn_mm_btn_info);
M2_SPACE(el_mm_btn_space, "W1h1");
M2_LIST(list_mm_btns) = {&el_mm_btn_settings, &el_mm_btn_space, &el_mm_btn_info}; 
M2_HLIST(el_mm_btns, "x0y0", list_mm_btns); 

// the channel info control
M2_LABEL(el_mm_ch1_label, NULL, "CH1"); 
M2_U32NUM(el_mm_ch1_value, "a0r1c4", &channel_info.ch[0]); 
M2_LIST(el_mm_ch1_list) = {&el_mm_ch1_label, &el_mm_ch1_value}; 
M2_HLIST(el_mm_ch1, NULL, el_mm_ch1_list); 

M2_LABEL(el_mm_ch2_label, NULL, "CH2"); 
M2_U32NUM(el_mm_ch2_value, "a0r1c4", &channel_info.ch[1]); 
M2_LIST(el_mm_ch2_list) = {&el_mm_ch2_label, &el_mm_ch2_value}; 
M2_HLIST(el_mm_ch2, NULL, el_mm_ch2_list); 

M2_LABEL(el_mm_ch3_label, NULL, "CH3"); 
M2_U32NUM(el_mm_ch3_value, "a0r1c4", &channel_info.ch[2]); 
M2_LIST(el_mm_ch3_list) = {&el_mm_ch3_label, &el_mm_ch3_value}; 
M2_HLIST(el_mm_ch3, NULL, el_mm_ch3_list); 

M2_LABEL(el_mm_ch4_label, NULL, "CH4"); 
M2_U32NUM(el_mm_ch4_value, "a0r1c4", &channel_info.ch[3]);
M2_LIST(el_mm_ch4_list) = {&el_mm_ch4_label, &el_mm_ch4_value}; 
M2_HLIST(el_mm_ch4, NULL, el_mm_ch4_list); 
 
M2_LABEL(el_mm_ch5_label, NULL, "CH5"); 
M2_U32NUM(el_mm_ch5_value, "a0r1c4", &channel_info.ch[4]); 
M2_LIST(el_mm_ch5_list) = {&el_mm_ch5_label, &el_mm_ch5_value}; 
M2_HLIST(el_mm_ch5, NULL, el_mm_ch5_list); 

M2_LABEL(el_mm_ch6_label, NULL, "CH6"); 
M2_U32NUM(el_mm_ch6_value, "a0r1c4", &channel_info.ch[5]); 
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

M2_LABEL(el_mm_chi_list_label, "-1", "\x1b[30;47m sticks \x1b[m"); 
M2_LIST(el_mm_chi_list) = {
		&el_mm_chi_list_label, 
		&el_mm_ch1, 
		&el_mm_ch2, 
		&el_mm_ch3, 
		&el_mm_ch4
}; 
M2_VLIST(el_mm_chi, "x0y1l5w15", el_mm_chi_list); 


M2_LABEL(el_mm_swi_list_label, "-1", "\x1b[30;47m  aux   \x1b[m"); 
M2_LIST(el_mm_swi_list) = {
		&el_mm_swi_list_label, 
		&el_mm_ch5, 
		&el_mm_ch6, 
		&el_mm_swab, 
		&el_mm_swcd
}; 
M2_VLIST(el_mm_swi, "x10y1l5w15", el_mm_swi_list); 

// place the title and the buttons and other controls into a vertical list
M2_LIST(list_mm_dialog) = {&el_mm_title, &el_mm_chi, &el_mm_swi, &el_mm_btns}; 
M2_XYLIST(el_mm_dialog, "l8", list_mm_dialog); 

M2_ALIGN(top_el_tlsm, "W64H64", &el_mm_dialog);
