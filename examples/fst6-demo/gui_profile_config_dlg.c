
M2_EXTERN_ALIGN(top_el_prof); 

M2_LABEL(el_prof_sound_label, "", "Sound"); 
M2_COMBO(el_prof_sound, NULL, &model.profile.sound, 2, fn_on_off_combo);
M2_LABEL(el_prof_backlight_label, "", "LCD backlight"); 
M2_COMBO(el_prof_backlight, NULL, &model.profile.lcd_backlight, 2, fn_on_off_combo);
M2_LABEL(el_prof_brightness_label, "", "LCD contrast"); 
M2_U8NUM(el_prof_brightness, "c3", 0, 100, &model.profile.lcd_brightness); 
M2_LABEL(el_prof_channels_label, "", "Outputs"); 
M2_ROOT(el_prof_channels, "", " >>> ", &top_el_chsm); 
M2_LABEL(el_prof_mixes_label, "", "Mixes"); 
M2_ROOT(el_prof_mixes, "", " >>> ", &top_el_mix_settings); 

M2_ROOT(el_prof_back_btn, "x1y1f4-1w44", " <<< ", &top_el_tlsm);
//M2_LIST(list_chsm_btns) = {&el_chsm_btn_back, &el_chsm_btn_space, &el_chsm_btn_save}; 
//M2_HLIST(el_chsm_btns, "x0y0", list_chsm_btns); 

// =================
M2_LIST(list_prof_gridlist) = {
	&el_prof_channels_label, &el_prof_channels, 
	&el_prof_mixes_label, &el_prof_mixes,
	&el_prof_sound_label, &el_prof_sound,
	&el_prof_backlight_label, &el_prof_backlight, 
	&el_prof_brightness_label, &el_prof_brightness
}; 
M2_GRIDLIST(el_prof_gridlist, "x4y16c2", list_prof_gridlist); 

void fn_prof_reset(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
	model.profile.do_reset = 1;
	m2_SetRoot(&top_el_prof); 
}

M2_LABEL(el_prof_cr_label, "", "Reset profile to defaults?"); 
M2_ALIGN(el_prof_cr_title, "-1|1W64H64", &el_prof_cr_label); 
M2_ROOT(el_prof_cr_btn_cancel, "x1y1f4w30", "   cancel ", &top_el_prof); 
M2_BUTTON(el_prof_cr_btn_ok, "x90y1f4w30", "     ok  ", fn_prof_reset); 
M2_LIST(list_prof_cr) = {&el_prof_cr_title, &el_prof_cr_btn_cancel, &el_prof_cr_btn_ok}; 
M2_XYLIST(top_el_prof_cr, "", list_prof_cr); 

M2_BUTTON(el_prof_calibrate_btn, "f4w35", " calibrate ", NULL);
M2_SPACE(el_prof_btns_space, "W1H1");  
M2_ROOT(el_prof_reset_btn, "f4w35", " reset ", &top_el_prof_cr); 
M2_LIST(list_prof_actions) = { 
	//&el_prof_calibrate_btn, 
	//&el_prof_btns_space, 
	&el_prof_reset_btn 
}; 
M2_GRIDLIST(el_prof_actions, "x81y16c1", list_prof_actions); 

M2_LABEL(el_prof_title_label, "", "== Profile settings ==");
M2_ALIGN(el_prof_title, "x0y58-1|0W64", &el_prof_title_label);

const char *fn_prof_select(uint8_t idx){
	static uint8_t prev_sel = 0; 
	if(prev_sel != model.profile.id){
		model.profile.request_load = 1; 
		prev_sel = model.profile.id; 
	}
	switch(idx){
		case 0: return "m1"; 
		case 1: return "m2"; 
		case 2: return "m3"; 
		case 3: return "m4"; 
		case 4: return "m5"; 
		case 5: return "m6"; 
	}
	return " "; 
}

M2_LABEL(el_prof_select_label, "", "Select profile: "); 
M2_COMBO(el_prof_combo, "x55y50", &model.profile.id, 6, fn_prof_select);
M2_LIST(list_prof_select) = {&el_prof_select_label, &el_prof_combo}; 
M2_HLIST(el_prof_select, "x30y50", list_prof_select); 

M2_LIST(list_mc) = {
	&el_prof_title,
	&el_prof_select, 
	&el_prof_gridlist,
	&el_prof_actions,  
	&el_prof_back_btn
}; 

M2_XYLIST(el_prof_list, "", list_mc); 
M2_ALIGN(top_el_prof, "x0y0-0|0W64H64", &el_prof_list); 
