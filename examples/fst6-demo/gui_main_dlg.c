// MEANT TO BE DIRECTLY INCLUDED INTO THE MAIN GUI FILE


/****************
* MAIN SCREEN 
*****************/
static char el_mm_btn_m_str[6][5]; 
void __attribute__((constructor)) fn_el_mm_btn_m_str_ctor(void){
	int c = 0; 
	for(c = 0; c < 6; c++)
		sprintf(el_mm_btn_m_str[c], " m%d ", c+1); 
	sprintf(el_mm_btn_m_str[0], "[m1]"); 
	sprintf(model.profile.name, "m1"); 
}

void fn_mm_btn_m(int c){
	int j; 
	sprintf(model.profile.name, "m%d",c+1); 
	for(j = 0; j < 6; j++) sprintf(el_mm_btn_m_str[j], " m%d ", j+1); 
	sprintf(el_mm_btn_m_str[c], "[m%d]", c+1); 
	model.profile.id = c;
	model.profile.request_load = 1; 
}

void fn_mm_btn_m1(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_mm_btn_m(0); 
}
void fn_mm_btn_m2(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_mm_btn_m(1); 
}
void fn_mm_btn_m3(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_mm_btn_m(2); 
}
void fn_mm_btn_m4(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_mm_btn_m(3); 
}
void fn_mm_btn_m5(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_mm_btn_m(4); 
}
void fn_mm_btn_m6(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_mm_btn_m(5); 
}

// the top model buttons

M2_BUTTON(el_mm_btn_m1, "", el_mm_btn_m_str[0], fn_mm_btn_m1); 
M2_BUTTON(el_mm_btn_m2, "", el_mm_btn_m_str[1], fn_mm_btn_m2); 
M2_BUTTON(el_mm_btn_m3, "", el_mm_btn_m_str[2], fn_mm_btn_m3); 
M2_BUTTON(el_mm_btn_m4, "", el_mm_btn_m_str[3], fn_mm_btn_m4); 
M2_BUTTON(el_mm_btn_m5, "", el_mm_btn_m_str[4], fn_mm_btn_m5); 
M2_BUTTON(el_mm_btn_m6, "", el_mm_btn_m_str[5], fn_mm_btn_m6); 
M2_LIST(list_mm_top_btns) = {&el_mm_btn_m1, &el_mm_btn_m2, &el_mm_btn_m3, &el_mm_btn_m4, &el_mm_btn_m5, &el_mm_btn_m6}; 
M2_HLIST(el_mm_top_btns, "x18y57", list_mm_top_btns); 
//M2_LIST(el_mm_top_btns_list) = {&el_mm_top_btns_h}; 


void fn_mm_btn_settings(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_sm);
}
void fn_mm_btn_info(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_im);
}
void fn_mm_btn_config(m2_el_fnarg_p fnarg) {
  (void)fnarg; 
  m2_SetRoot(&top_el_prof);
}

M2_BUTTON(el_mm_btn_settings, "-1w30f4", "  about ", fn_mm_btn_info);
M2_BUTTON(el_mm_btn_info, "-1w30f4", "      >>>  ", fn_mm_btn_settings);
M2_BUTTON(el_mm_btn_config, "-1w29f4", "    conf  ", fn_mm_btn_config);
M2_LABEL(el_mm_btn_space, "w53", "");
M2_LIST(list_mm_btns) = {
	&el_mm_btn_settings, 
	&el_mm_btn_space, 
	&el_mm_btn_config, 
	//&el_mm_btn_space, 
	//&el_mm_btn_info
}; 
M2_HLIST(el_mm_btns, "c3W64", list_mm_btns); 

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

M2_LABEL(el_mm_ch1_label, "", "OUT1"); 
M2_U32NUM(el_mm_ch1_value, "r1c4", &model.out[0].value); 

void fn_mm_btn_ch2(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(1); 
}

M2_LABEL(el_mm_ch2_label, "", "OUT2"); 
M2_U32NUM(el_mm_ch2_value, "r1c4", &model.out[1].value); 


void fn_mm_btn_ch3(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(2); 
}

M2_LABEL(el_mm_ch3_label, "", "OUT3"); 
M2_U32NUM(el_mm_ch3_value, "r1c4", &model.out[2].value); 

void fn_mm_btn_ch4(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(3); 
}

M2_LABEL(el_mm_ch4_label, "", "OUT4"); 
M2_U32NUM(el_mm_ch4_value, "r1c4", &model.out[3].value);
 
void fn_mm_btn_ch5(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(4); 
}

M2_LABEL(el_mm_ch5_label, "", "OUT5"); 
M2_U32NUM(el_mm_ch5_value, "r1c4", &model.out[4].value); 

void fn_mm_btn_ch6(m2_el_fnarg_p fnarg) { 
	(void)fnarg; 
	fn_chsm_go(5); 
}

M2_LABEL(el_mm_ch6_label, "", "OUT6"); 
M2_U32NUM(el_mm_ch6_value, "r1c4", &model.out[5].value); 

M2_LIST(list_mm_chi) = {
		&el_mm_ch1_label, &el_mm_ch1_value,
		&el_mm_ch2_label, &el_mm_ch2_value, 
		&el_mm_ch3_label, &el_mm_ch3_value, 
		&el_mm_ch4_label, &el_mm_ch4_value,
		&el_mm_ch5_label, &el_mm_ch5_value, 
		&el_mm_ch6_label, &el_mm_ch6_value,
}; 
M2_GRIDLIST(el_mm_chi, "x8y17c2W25", list_mm_chi); 

// MIX SELECITON PART
	
	#define MULTI_SELECT_CNT 3
	const char *multi_select_strings[MULTI_SELECT_CNT] = { "mix 1", "mix 2", "mix 3" };
	
	uint8_t el_muse_first = 0;
	uint8_t el_muse_cnt = MULTI_SELECT_CNT;

	const char *el_muse_strlist_cb(uint8_t idx, uint8_t msg) {
		const char *s = "-none-";
		if ( msg == M2_STRLIST_MSG_SELECT ) {
			if ( model.profile.mix_enabled[idx] == 0 ) {
				model.profile.mix_enabled[idx] = 1;
			}
			else {
				model.profile.mix_enabled[idx] = 0;
			}
		}
		if ( msg == M2_STRLIST_MSG_GET_STR ) {
			s = multi_select_strings[idx];
		}
		if ( msg == M2_STRLIST_MSG_GET_EXTENDED_STR ) {
			if ( model.profile.mix_enabled[idx] == 0 ) {
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
	M2_STRLIST(el_mixes_strlist, "l3e3w19", &el_muse_first, &el_muse_cnt, el_muse_strlist_cb);
	
	M2_BUTTON(el_mix_1_btn, "", " >>> ", fn_mix_1_btn); 
	M2_BUTTON(el_mix_2_btn, "", " >>> ", fn_mix_2_btn); 
	M2_BUTTON(el_mix_3_btn, "", " >>> ", fn_mix_3_btn); 
	M2_LIST(list_mm_mix_btns) = {
		&el_mix_1_btn, &el_mix_2_btn, &el_mix_3_btn
	}; 
	M2_VLIST(el_mm_mix_btns, "l3", list_mm_mix_btns); 
	
	M2_LIST(list_mm_mix) = {
		//&el_muse_strlist, 
		&el_mm_mix_btns
	}; 
	M2_HLIST(el_mm_mix_list, "x81y17l3", list_mm_mix); 
	
	const char *fn_mm_armed(uint8_t idx){
		switch(idx){
			case 0: return "ARMED"; 
			case 1: return "OFF"; 
		}
		return " "; 
	}

	M2_COMBO(el_mm_armed_combo, "f2x87y42", &model.armed, 2, fn_mm_armed);
	
	M2_U32NUM(el_mm_vbat_value, "r1c3f2.1", &model.vbat); 
	M2_LABEL(el_mm_vbat_volt, "f2", "V"); 
	M2_LIST(list_mm_vbat) = {&el_mm_vbat_value, &el_mm_vbat_volt}; 
	M2_HLIST(el_mm_vbat, "x86y42", list_mm_vbat); 
	/*
	const char *fn_mm_profile_select(uint8_t idx){
		switch(idx){
			case 0: return "no"; 
			case 1: return "yes"; 
		}
		return " "; 
	}
	M2_COMBO(el_chsm_reverse, NULL, &model.profile.id, 6, fn_mm_profile_select);
*/
	//M2_LABEL(el_mm_prof_name, "x60y2", model.profile.name); 
	
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

M2_LABEL(el_mm_fst6_label, "x51y44f1", "KT-6");
M2_SPACE(el_mm_fst6_space, "w1h2");

GUI_SWITCH(el_mm_swa_value, "w4h6b1", &model.sw[0]); 
GUI_SWITCH(el_mm_swb_value, "w4h6b1", &model.sw[1]); 
GUI_SWITCH(el_mm_swc_value, "w4h6b1", &model.sw[2]); 
GUI_SWITCH(el_mm_swd_value, "w4h6b1", &model.sw[3]); 
M2_LIST(list_mm_sw) = {
	&el_mm_swd_value, &el_mm_swb_value, &el_mm_swa_value, &el_mm_swc_value
}; 
M2_HLIST(el_mm_sw_list, "c4", list_mm_sw); 

M2_LIST(list_mm_middle) = {
	&el_mm_fst6_space,
	&el_mm_sw_list, 
	&el_mm_fst6_space,
	&el_mixes_strlist, 
	//&el_mm_calibrate, 
	//&el_mm_fst6_space, 
	//&el_mm_reset
}; 
M2_VLIST(el_mm_center, "x52y14c1", list_mm_middle); 

// horizontal left
GUI_CHANNEL(el_mm_ch3_sl, "x2y14w2h40", &chmin, &chmax, &chcenter, &model.out[2].value); 
GUI_CHANNEL(el_mm_ch4_sl, "x6y12w40h2", &chmin, &chmax, &chcenter, &model.out[3].value); 

GUI_CHANNEL(el_mm_ch2_sl, "x123y14w2h40", &chmin, &chmax, &chcenter, &model.out[1].value); 
GUI_CHANNEL(el_mm_ch1_sl, "x81y12w40h2", &chmin, &chmax, &chcenter, &model.out[0].value); 

GUI_CHANNEL(el_mm_ch5_sl, "x77y14w2h40", &chmin, &chmax, &chcenter, &model.out[4].value); 
GUI_CHANNEL(el_mm_ch6_sl, "x48y14w2h40", &chmin, &chmax, &chcenter, &model.out[5].value); 

GUI_FRAME(el_mm_frame, "x0y0"); 

M2_LIST(list_mm_channel_values) = {
	&el_mm_ch1_sl, &el_mm_ch2_sl, &el_mm_ch3_sl, &el_mm_ch4_sl, &el_mm_ch5_sl, &el_mm_ch6_sl
}; 
M2_XYLIST(el_mm_channel_values, "W64H64", list_mm_channel_values); 

// place the title and the buttons and other controls into a vertical list
M2_LIST(list_mm_dialog) = {
		&el_mm_frame, 
		//&el_mm_prof_name, 
		&el_mm_top_btns, 
		&el_mm_fst6_label, 
		&el_mm_chi, 
		&el_mm_center, 
		&el_mm_vbat, 
		//&el_mm_armed_combo,
		//&el_mm_mix_list,
		&el_mm_btns,
		&el_mm_channel_values, 
		//&el_mm_swi
}; 
M2_XYLIST(top_el_tlsm, "", list_mm_dialog); 
