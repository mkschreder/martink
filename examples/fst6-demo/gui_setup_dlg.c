 
/****************
* SETTINGS MENU
*****************/

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

M2_COMBO(el_sm_mode, NULL, &model.profile.id, 4, fn_sm_mode_select);
//===============
// CHANNELS 
	
	// ===============

M2_LABEL(el_sm_title_label, "", "===== settings ======");
M2_ALIGN(el_sm_title, "x0y53-1W64", &el_sm_title_label);

// options
M2_LABEL(el_sm_name_label, "", "Name"); 
M2_TEXT(el_sm_name, "a0", model.profile.name, sizeof(model.profile.name)); 

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
