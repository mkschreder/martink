
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

M2_LABEL(el_chsm_title_label, "", "== Channel settings ==");
M2_ALIGN(el_chsm_title, "x8y58-1|0W64", &el_chsm_title_label);

M2_LABEL(el_chsm_channel_label, "", "Select channel: "); 
const char *fn_channel_select(uint8_t idx){
	static uint8_t prev_sel = 0; 
	if(prev_sel != model.channel.id){
		model.channel.request_load = 1; 
		prev_sel = model.channel.id; 
	}
	return fn_channel_picker_combo(idx); 
}
M2_COMBO(el_chsm_channel, "x58y57", &model.channel.id, 6, fn_channel_select);
M2_LIST(list_chsm_channel_select) = {&el_chsm_channel_label, &el_chsm_channel}; 
M2_HLIST(el_chsm_channel_select, "x30y50", list_chsm_channel_select); 

M2_LABEL(el_chsm_source_label, "", "Source"); 
M2_COMBO(el_chsm_source, NULL, &model.channel.source, 10, fn_chsm_source);

M2_LABEL(el_chsm_reverse_label, "", "Reverse"); 
const char *fn_chsm_reverse_select(uint8_t idx){
	switch(idx){
		case 0: return "no"; 
		case 1: return "yes"; 
	}
	return " "; 
}
M2_COMBO(el_chsm_reverse, NULL, &model.channel.reverse, 2, fn_chsm_reverse_select);

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
M2_U32NUM(el_chsm_out, "x81y40f1c4", &model.channel.output); 
// =================
// BOTTOM LINE
M2_EXTERN_ALIGN(top_el_sm); 
M2_EXTERN_ALIGN(top_el_prof); 

void fn_chsm_btn_save(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
	//fn_chsm_save_channel();
	//m2_SetRoot(&top_el_sm);
}

M2_ROOT(el_chsm_back, "x1y1f4-1w44", " <<< ", &top_el_prof);
//M2_LIST(list_chsm_btns) = {&el_chsm_btn_back, &el_chsm_btn_space, &el_chsm_btn_save}; 
//M2_HLIST(el_chsm_btns, "x0y0", list_chsm_btns); 

M2_LIST(list_chsm_left_list) = {
	//&el_chsm_channel_label, &el_chsm_channel, 
	&el_chsm_source_label, &el_chsm_source,
	&el_chsm_raw_label, &el_chsm_raw, 
	&el_chsm_reverse_label, &el_chsm_reverse, 
	&el_chsm_rate_label, &el_chsm_rate,  
	//&el_chsm_out_label, &el_chsm_out
}; 
M2_GRIDLIST(el_chsm_left_list, "x10y13c2", list_chsm_left_list); 

M2_LIST(list_chsm_right_list) = {
	//&el_chsm_channel_label, &el_chsm_channel,  
	&el_chsm_exp_label, &el_chsm_exp, 
	&el_chsm_min_label, &el_chsm_min, 
	&el_chsm_max_label, &el_chsm_max, 
	&el_chsm_offset_label, &el_chsm_offset, 
	//&el_chsm_out_label, &el_chsm_out
}; 
M2_GRIDLIST(el_chsm_right_list, "x75y13c2", list_chsm_right_list); 

GUI_CHANNEL(el_chsm_out_indicator, "x2y11w4h50", &chmin, &chmax, &chcenter, &model.channel.output); 

// ===============
M2_LIST(list_chsm) = {
	//&el_mm_channel_values, 
	//&el_mm_frame, 
	//&el_chsm_top_btns, 
	&el_chsm_title, 
	&el_chsm_out_indicator, 
	&el_chsm_channel_select,
	&el_chsm_left_list, 
	&el_chsm_right_list, 
	&el_chsm_back
}; 
M2_XYLIST(el_chsm_list, "", list_chsm); 
M2_ALIGN(top_el_chsm, "x0y0-0|0W64H64", &el_chsm_list); 
