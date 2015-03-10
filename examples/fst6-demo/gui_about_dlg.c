 

/****************
* ABOUT MENU
*****************/

M2_LABEL(el_im_title_label, "-1", "about");
uint8_t total_lines1 = 8;
uint8_t first_visible_line1 = 0;

void fn_about_back(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
  m2_SetRoot(&top_el_tlsm);
}

M2_INFO(el_info1, "x0y10W64l10", &first_visible_line1, &total_lines1, 
	"                           ABOUT\n"\
	"              FS-T6 Firmware KT-6\n\n"\
	"Author: Martin K. Schroder\n"\
	"Copyright: 2015, GPL\n\n"\
	"This program is free software\n\n"\
	"               Click OK to go back", fn_about_back);
	
M2_INFO(top_fw_upgrade, "x0y10W64l10", &first_visible_line1, &total_lines1, 
	"                  Firmware Upgrade\n\n"\
	"Now in firmware upgrade mode. \n\n"\
	"Please reboot after the firmware \n"\
	"upgrade is completed\n", NULL);
	
void fn_firmware_upgrade(m2_el_fnarg_p fnarg) {
	(void)fnarg; 
	model.do_flash_upgrade = 1; 
  m2_SetRoot(&top_fw_upgrade);
}

M2_BUTTON(el_fw_upgrade_btn, "f4", " Firmware Upgrade ", fn_firmware_upgrade); 
M2_ALIGN(el_fw_upgrade, "-1|0W64H64x0y0", &el_fw_upgrade_btn); 

//M2_VSB(el_vsb1, "W2r1l10", &first_visible_line1, &total_lines1);
M2_LIST(el_list1) = { &el_info1, &el_fw_upgrade};


M2_XYLIST(top_el_im, NULL, el_list1);
