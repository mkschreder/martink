 
M2_LABEL(el_mixsm_master_label, "", "Master"); 
M2_COMBO(el_mixsm_master, NULL, &model.mix.master_channel, 6, fn_channel_picker_combo);
M2_LABEL(el_mixsm_slave_label, "", "Slave"); 
M2_COMBO(el_mixsm_slave, NULL, &model.mix.slave_channel, 6, fn_channel_picker_combo);

M2_LABEL(el_mixsm_pos_label, "", "Pos. Mix"); 
M2_U8NUM(el_mixsm_pos, "c3", 0, 100, &model.mix.pos_mix); 
M2_LABEL(el_mixsm_neg_label, "", "Neg. Mix"); 
M2_U8NUM(el_mixsm_neg, "c3", 0, 100, &model.mix.neg_mix); 
M2_LABEL(el_mixsm_offset_label, "", "Offset"); 
M2_U8NUM(el_mixsm_offset, "c3", 0, 100, &model.mix.offset);
M2_LABEL(el_mixsm_enabled_label, "", "Enabled"); 
M2_COMBO(el_mixsm_enabled, "", &model.mix.enabled, 2, fn_on_off_combo); 

M2_ROOT(el_mixsm_back_btn, "x1y1f4-1w44", " <<< ", &top_el_prof);
//M2_LIST(list_chsm_btns) = {&el_chsm_btn_back, &el_chsm_btn_space, &el_chsm_btn_save}; 
//M2_HLIST(el_chsm_btns, "x0y0", list_chsm_btns); 

// =================
M2_LIST(list_mixsm_gridlist) = {
	&el_mixsm_enabled_label, &el_mixsm_enabled, 
	&el_mixsm_master_label, &el_mixsm_master,
	&el_mixsm_slave_label, &el_mixsm_slave, 
	&el_mixsm_pos_label, &el_mixsm_pos, 
	&el_mixsm_neg_label, &el_mixsm_neg,  
	//&el_mixsm_offset_label, &el_mixsm_offset
}; 
M2_GRIDLIST(el_mixsm_gridlist, "x4y16c2", list_mixsm_gridlist); 

M2_LABEL(el_mixsm_title_label, "", "== Mix settings ==");
M2_ALIGN(el_mixsm_title, "x0y58-1|0W64", &el_mixsm_title_label);

const char *fn_mixsm_mix_select(uint8_t idx){
	static uint8_t prev_sel = 0; 
	if(prev_sel != model.mix.id){
		model.mix.request_load = 1; 
		prev_sel = model.mix.id; 
	}
	switch(idx){
		case 0: return "mix 1"; 
		case 1: return "mix 2"; 
		case 2: return "mix 3"; 
	}
	return " "; 
}

M2_LABEL(el_mixsm_select_label, "", "Select profile: "); 
M2_COMBO(el_mixsm_mix_combo, "x55y50", &model.mix.id, 3, fn_mixsm_mix_select);
M2_LIST(list_mixsm_mix_select) = {&el_mixsm_select_label, &el_mixsm_mix_combo}; 
M2_HLIST(el_mixsm_mix_select, "x30y50", list_mixsm_mix_select); 

GUI_CHANNEL(el_mixsm_master_ind, "x60y41w40h4", &chmin, &chmax, &chcenter, &model.mix.master_value); 
GUI_CHANNEL(el_mixsm_slave_ind, "x60y35w40h4", &chmin, &chmax, &chcenter, &model.mix.slave_value); 

M2_LIST(list_mixsm) = {
	&el_mixsm_title,
	&el_mixsm_mix_select, 
	&el_mixsm_master_ind, 
	&el_mixsm_slave_ind,
	&el_mixsm_gridlist, 
	&el_mixsm_back_btn
}; 
M2_XYLIST(el_mixsm_list, "", list_mixsm); 
M2_ALIGN(top_el_mix_settings, "x0y0-0|0W64H64", &el_mixsm_list); 
