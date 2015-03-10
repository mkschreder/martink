#pragma once


struct gui_channel_settings {
	uint8_t id; 
	uint32_t input; 
	uint32_t min; 
	uint32_t max; 
	uint8_t reverse; 
	uint8_t rate; 
	uint8_t exponent; 
	uint8_t source; 
	uint32_t offset; 
	uint8_t request_load; 
	uint32_t output; 
}; 

struct gui_switch {
	uint8_t on; 
};

struct gui_profile_settings {
	uint8_t id; 
	char name[8]; 
	uint8_t mix_enabled[3]; 
	uint8_t sound; 
	uint8_t lcd_brightness; 
	uint8_t lcd_backlight; 
	uint8_t request_load; 
	uint8_t do_reset; 
}; 

struct gui_output_channel {
	uint32_t value; 
}; 

struct gui_input_channel {
	uint32_t value; 
}; 

struct gui_mix {
	uint8_t id; 
	uint8_t master_channel; 
	uint8_t slave_channel; 
	uint32_t master_value; 
	uint32_t slave_value; 
	uint8_t pos_mix; 
	uint8_t neg_mix; 
	uint8_t offset; 
	uint8_t enabled; 
	uint8_t request_load; 
}; 

struct gui_data {
	struct gui_output_channel out[6]; 
	struct gui_input_channel inputs[6]; 
	uint8_t sw[4]; 
	struct gui_profile_settings profile; // current profile
	struct gui_channel_settings channel; // current channel
	struct gui_mix mix; // currently edited mix
	uint8_t armed; 
	uint32_t vbat; 
	uint8_t do_flash_upgrade; 
}; 
